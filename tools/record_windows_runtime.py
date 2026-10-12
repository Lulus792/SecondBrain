#!/usr/bin/env python3
"""Record map-named SDK libraries that MSVC actually reports loading."""
import argparse
import hashlib
import json
from pathlib import Path, PureWindowsPath
import re
import sys

from record_linux_startup import checked_map, digest

SDK_LIBRARIES = {'libcmt', 'libcmtd', 'libucrt', 'libucrtd',
                 'libvcruntime', 'libvcruntimed', 'uuid'}
SDK_VARIANTS = ({'libcmt', 'libcmtd'}, {'libucrt', 'libucrtd'},
                {'libvcruntime', 'libvcruntimed'}, {'uuid'})


def loaded_libraries(text):
    searches, loads = {}, []
    for line in text.splitlines():
        search = re.search(r'\bSearching\s+(.+?\.lib):\s*$', line, re.I)
        if search:
            path = PureWindowsPath(search[1].strip().strip('"'))
            if path.is_absolute():
                searches.setdefault(path.stem.lower(), set()).add(str(path))
        loaded = re.search(r'\bLoaded\s+(.+?\.lib)\(([^()]+)\)\s*$', line, re.I)
        if loaded:
            loads.append((PureWindowsPath(loaded[1].strip().strip('"')), loaded[2]))
    result = {}
    for path, member in loads:
        name = path.stem.lower()
        if name not in SDK_LIBRARIES:
            continue
        if path.is_absolute():
            resolved = str(path)
        else:
            choices = searches.get(name, set())
            if len(choices) != 1:
                raise ValueError('Loaded SDK library lacks one unambiguous full search path: ' + name)
            resolved = next(iter(choices))
        result.setdefault(name, {}).setdefault(resolved, set()).add(member)
    if not result:
        raise ValueError('No SDK libraries actually reported loaded')
    return result


def map_members(text):
    result = {}
    for library, member in re.findall(r'\s([^\s:]+):([^\s]+\.obj)\s*$', text, re.M):
        name = library.lower().removesuffix('.lib')
        if name in SDK_LIBRARIES:
            result.setdefault(name, set()).add(member)
    if not result:
        raise ValueError('No named SDK object members in MSVC map')
    return result


def dll_repro(program, map_path, directory, libraries):
    identity = checked_map(program, map_path)
    members = map_members(map_path.read_text())
    directory = directory.resolve(strict=True)
    references = []
    for name, objects in members.items():
        copies = [p for p in directory.rglob('*')
                  if p.is_file() and p.suffix.lower() == '.lib' and p.stem.lower() == name]
        if len(copies) != 1:
            raise ValueError('Expected one captured DLL SDK library: ' + name)
        copy = copies[0].resolve(strict=True)
        if not copy.is_relative_to(directory):
            raise ValueError('Captured SDK library escapes the link repro')
        captured_hash = digest(copy)
        candidates = {path: record['sha256'] for path, record in libraries.items()
                      if record['library'] == name}
        family = next(group for group in SDK_VARIANTS if name in group)
        for path, record in libraries.items():
            if record['library'] in family and record['library'] != name:
                # Cargo builds its DLL in Release even in the Debug UI job.
                # Compare the requested counterpart's actual bytes in the
                # observed SDK directory; never substitute the debug archive.
                counterpart = Path(path).with_name(name + '.lib')
                if counterpart.is_file():
                    candidates[str(counterpart.resolve())] = digest(counterpart)
        originals = [path for path, value in candidates.items() if value == captured_hash]
        if len(originals) != 1:
            raise ValueError('Captured DLL SDK bytes lack one matching SDK inventory file: ' + name)
        original = originals[0]
        if digest(Path(original)) != captured_hash or digest(copy) != captured_hash:
            raise ValueError('SDK file or captured copy changed during DLL inventory')
        references.append({'library': name, 'captured_file': str(copy),
                           'sha256': captured_hash, 'byte_identical_sdk_file': original,
                           'named_object_members': sorted(objects)})
    if checked_map(program, map_path) != identity:
        raise ValueError('DLL/map changed during inventory')
    for record in references:
        if (digest(Path(record['captured_file'])) != record['sha256'] or
                digest(Path(record['byte_identical_sdk_file'])) != record['sha256']):
            raise ValueError('SDK library or captured copy changed during inventory')
    return {'scope': 'dll-map-named-members-and-link-repro-library-bytes-matched-to-sdk-inventory',
            'identity': identity, 'sdk_libraries': references,
            'limitation': 'Byte equality identifies SDK contents; it does not independently prove the original path selected by the DLL linker'}


def collect(build, config, log):
    log_bytes = log.read_bytes()
    loaded = loaded_libraries(log_bytes.decode('utf-8'))
    result = {'format': 1, 'scope': 'msvc-map-named-runtime-library-files-observed-loaded-in-completed-build',
              'build_log_sha256': hashlib.sha256(log_bytes).hexdigest(), 'programs': {}, 'libraries': {}}
    for target in ('secondbrain', 'secondbrain-cli'):
        map_path = build / 'link-maps' / config / (target + '.map')
        candidates = [p for p in (build/config/(target+'.exe'), build/(target+'.exe')) if p.is_file()]
        matching = []
        for path in candidates:
            try:
                matching.append((path, checked_map(path, map_path)))
            except ValueError:
                pass
        if len(matching) != 1:
            raise ValueError('Expected one executable matching its map identity: ' + target)
        program, identity = matching[0]
        members = map_members(map_path.read_text())
        references = []
        for name, names in members.items():
            choices = loaded.get(name, {})
            if len(choices) != 1:
                raise ValueError('Mapped SDK library has no unique loaded file: ' + name)
            original, loaded_members = next(iter(choices.items()))
            # Both programs share a completed build log; require every named
            # member to have actually been loaded from the unique SDK file.
            if not names.issubset(loaded_members):
                raise ValueError('Mapped SDK members missing from completed linker log: ' + name)
            path = Path(original).resolve(strict=True)
            key = str(path)
            if key not in result['libraries']:
                result['libraries'][key] = {'sha256': digest(path), 'linker_path': original,
                                            'library': name}
            references.append({'path': key, 'named_object_members': sorted(names)})
        if checked_map(program, map_path) != identity:
            raise ValueError('Executable/map changed during inventory')
        result['programs'][target] = {'identity': identity, 'sdk_libraries': references}
    if log_bytes != log.read_bytes():
        raise ValueError('Build log changed during inventory')
    for path, record in result['libraries'].items():
        if digest(Path(path)) != record['sha256']:
            raise ValueError('SDK library changed during inventory')
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--build-dir', type=Path, required=True)
    parser.add_argument('--config', required=True)
    parser.add_argument('--log', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--accesskit-repro', type=Path)
    args = parser.parse_args()
    if sys.platform != 'win32':
        parser.error('SDK file inventory must run on Windows')
    if not re.fullmatch(r'[A-Za-z0-9_-]+', args.config):
        parser.error('Invalid configuration name')
    if args.output.exists():
        parser.error('Existing output is protected; choose a new report path')
    build = args.build_dir.resolve()
    report = collect(build, args.config, args.log)
    if args.accesskit_repro:
        target = build / 'accesskit-windows-build'
        program = target / 'x86_64-pc-windows-msvc/release/accesskit.dll'
        report['accesskit_dll_repro'] = dll_repro(program, target/'accesskit.map',
                                               args.accesskit_repro, report['libraries'])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('x', encoding='utf-8') as stream:
        json.dump(report, stream, ensure_ascii=False, indent=2)
        stream.write('\n')
    print('Windows runtime provenance:', len(report['libraries']), 'loaded SDK files')


if __name__ == '__main__':
    main()
