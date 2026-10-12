#!/usr/bin/env python3
"""Bind ELF startup contributors to installed Debian package provenance."""
import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import subprocess
import sys

STARTUP = {'Scrt1.o', 'crt1.o', 'rcrt1.o', 'crti.o', 'crtn.o',
           'crtbegin.o', 'crtbeginS.o', 'crtbeginT.o', 'crtend.o', 'crtendS.o'}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def startup_contributors(text):
    marker = 'Linker script and memory map\n'
    if text.count(marker) != 1:
        raise ValueError('Expected one GNU output-section map')
    found = {}
    section = ''
    for line in text.split(marker, 1)[1].splitlines():
        label = re.match(r'^\s*(\.[^\s]+)', line)
        if label:
            section = label[1]
        row = re.match(r'^\s*(?:(\.[^\s]+)\s+)?0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+(.+\.o)\s*$', line)
        if not row or int(row[3], 16) == 0:
            continue
        kind = row[1] or section
        if kind.startswith(('.debug', '.comment')):
            continue
        # Map paths describe the Linux target even when parser tests run on Windows.
        path = PurePosixPath(row[4])
        if path.name in STARTUP:
            if not path.is_absolute():
                raise ValueError('Startup object does not have an absolute path')
            found.setdefault(str(path), []).append({'section': kind, 'bytes': int(row[3], 16)})
    if not found:
        raise ValueError('No nonempty startup-object contributions found')
    return found


def command(*args):
    environment = dict(os.environ, LC_ALL='C.UTF-8')
    return subprocess.run(args, check=True, capture_output=True, text=True,
                          env=environment, timeout=30).stdout


def package_metadata(path, run=command):
    lines = run('dpkg-query', '-S', str(path)).splitlines()
    owners = {line.rsplit(': ', 1)[0] for line in lines
              if ': ' in line and line.rsplit(': ', 1)[1] == str(path)}
    if len(owners) != 1:
        raise ValueError('Startup object has no unique exact package owner: ' + str(path))
    owner = owners.pop()
    if not re.fullmatch(r'[a-z0-9][a-z0-9+.-]*(?::[a-z0-9-]+)?', owner):
        raise ValueError('Invalid package owner')
    fields = run('dpkg-query', '-W', '-f=${binary:Package}\t${Version}\t${source:Package}\t${source:Version}\t${Architecture}\n', owner).strip().split('\t')
    if len(fields) != 5 or not all(fields) or fields[0] != owner:
        raise ValueError('Incomplete installed package/source identity')
    return dict(zip(('binary_package', 'binary_version', 'source_package', 'source_version', 'architecture'), fields))


def checked_map(program, path):
    lines = Path(str(path) + '.identity').read_text().splitlines()
    if not lines or lines[0] != 'SecondBrain link map 1' or len(lines) != 3:
        raise ValueError('Invalid link-map identity')
    values = dict(line.split('=', 1) for line in lines[1:])
    if set(values) != {'program_sha256', 'map_sha256'}:
        raise ValueError('Invalid link-map identity fields')
    if values['program_sha256'] != digest(program) or values['map_sha256'] != digest(path):
        raise ValueError('Program or map no longer matches its build identity')
    return values


def collect(build, config):
    result = {'format': 1, 'scope': 'gnu-elf-nonempty-startup-contributions-and-installed-debian-packages',
              'programs': {}, 'objects': {}, 'copyright_notices': {}}
    for target in ('secondbrain', 'secondbrain-cli'):
        program, path = build / target, build / 'link-maps' / config / (target + '.map')
        identity = checked_map(program, path)
        contributions = startup_contributors(path.read_text())
        references = []
        for original, sections in contributions.items():
            real = Path(original).resolve(strict=True)
            key = str(real)
            if key not in result['objects']:
                before = digest(real)
                package = package_metadata(real)
                notice = Path('/usr/share/doc') / package['binary_package'].split(':')[0] / 'copyright'
                raw = notice.read_bytes()
                notice_hash = hashlib.sha256(raw).hexdigest()
                if before != digest(real) or notice_hash != digest(notice):
                    raise ValueError('Installed source evidence changed while reading')
                result['copyright_notices'][notice_hash] = {
                    'path': str(notice), 'resolved_path': str(notice.resolve()),
                    'sha256': notice_hash, 'text': raw.decode('utf-8')}
                result['objects'][key] = {'sha256': before, 'package': package,
                                          'copyright_notice_sha256': notice_hash}
            references.append({'map_path': original, 'resolved_path': key, 'contributions': sections})
        # The inventory remains tied to the original files after package reads.
        if checked_map(program, path) != identity:
            raise ValueError('Build identity changed during inventory')
        result['programs'][target] = {'identity': identity, 'startup_objects': references}
    for path, record in result['objects'].items():
        if digest(Path(path)) != record['sha256']:
            raise ValueError('Installed object changed during inventory')
    for record in result['copyright_notices'].values():
        if digest(Path(record['path'])) != record['sha256']:
            raise ValueError('Installed copyright notice changed during inventory')
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--build-dir', type=Path, required=True)
    parser.add_argument('--config', required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if sys.platform != 'linux':
        parser.error('The installed-package inventory must run on Linux')
    if not re.fullmatch(r'[A-Za-z0-9_-]+', args.config):
        parser.error('Invalid configuration name')
    if args.output.exists():
        parser.error('Existing output is protected; choose a new report path')
    report = collect(args.build_dir.resolve(), args.config)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('x', encoding='utf-8') as stream:
        json.dump(report, stream, ensure_ascii=False, indent=2)
        stream.write('\n')
    print('Linux startup provenance:', len(report['objects']), 'files;',
          len(report['copyright_notices']), 'installed copyright notices')


if __name__ == '__main__':
    main()
