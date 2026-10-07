#!/usr/bin/env python3
"""Collect pinned UI notices from Cargo metadata and original source trees.

Developer tool only; the application and package do not run Python.
"""
import argparse
import hashlib
import json
import re
import urllib.request
import ssl
from pathlib import Path


def digest(data):
    return hashlib.sha256(data).hexdigest()


def fetch(url):
    context = ssl.create_default_context(cafile='/etc/ssl/cert.pem' if Path('/etc/ssl/cert.pem').is_file() else None)
    try:
        return urllib.request.urlopen(url, context=context, timeout=30).read()
    except Exception as error:
        raise RuntimeError("Could not read pinned original: " + url) from error


def collect(args):
    import tomllib
    original = Path(args.accesskit_source)
    lock_bytes = (original / 'Cargo.lock').read_bytes()
    if digest(lock_bytes) != '97ac8966d2fadbc325ef960d874d89b50877253bcdcffaec499645f18b04ef7d':
        raise ValueError('Original AccessKit lock differs from the reviewed release')
    locked = {(p['name'], p['version']): p for p in tomllib.loads(lock_bytes.decode())['package']}
    components = {}
    payloads = {}

    def notice(data, origin, owner):
        data.decode('utf-8')
        key = digest(data)
        payload = payloads.setdefault(key, {'text': data.decode(), 'sources': [], 'components': []})
        if origin not in payload['sources']:
            payload['sources'].append(origin)
        if owner not in payload['components']:
            payload['components'].append(owner)
        return {'sha256': key, 'source': origin}

    for platform, filename in (pair.split('=', 1) for pair in args.metadata):
        metadata = json.loads(Path(filename).read_text(encoding='utf-8'))
        for package in metadata['packages']:
            name, version = package['name'], package['version']
            identity = name + '@' + version
            if (name, version) not in locked:
                raise ValueError('Package not in pinned original lock: ' + identity)
            root = Path(package['manifest_path']).parent
            record = components.setdefault(identity, {
                'name': name, 'version': version, 'declared_license': package['license'],
                'archive_sha256': locked[(name, version)].get('checksum'),
                'repository': package['repository'], 'platforms': [], 'notices': [], 'copyrights': []})
            record['platforms'].append(platform)
            if record['notices']:
                continue
            checksum_path = root / '.cargo-checksum.json'
            checksums = json.loads(checksum_path.read_text(encoding='utf-8')) if checksum_path.exists() else None
            if checksums and record['archive_sha256'] and checksums['package'] != record['archive_sha256']:
                raise ValueError('Crate archive checksum differs: ' + identity)
            candidates = sorted(p for p in root.iterdir() if p.is_file() and
                                re.match(r'(?i)^(LICENSE|COPYING|NOTICE|COPYRIGHT|AUTHORS)', p.name))
            # This LGPL text belongs to Meson, which our Cargo build does not use.
            candidates = [p for p in candidates if not (name == 'accesskit-c' and p.name == 'COPYING.LIB')]
            for path in candidates:
                data = path.read_bytes()
                if checksums and path.name in checksums['files'] and digest(data) != checksums['files'][path.name]:
                    raise ValueError('Changed original notice: ' + str(path))
                origin = ('https://github.com/AccessKit/accesskit-c/blob/8b6ed37c20ed4c59390e253407983333053662ba/' + path.name
                          if name == 'accesskit-c' else 'https://crates.io/crates/' + name + '/' + version + ' : ' + path.name)
                record['notices'].append(notice(data, origin, identity))
            if not candidates:
                vcs = json.loads((root / '.cargo_vcs_info.json').read_text(encoding='utf-8'))
                commit = vcs['git']['sha1']
                repo = package['repository'].removesuffix('.git')
                if repo not in ('https://github.com/AccessKit/accesskit', 'https://github.com/madsmtm/objc2'):
                    raise ValueError('Missing notice needs manual original-source review: ' + identity)
                names = ['LICENSE-MIT', 'LICENSE-APACHE'] if 'AccessKit' in repo else ['LICENSE.md' if commit == '8d214f5477365ffcbcbb7de058c86ed9a518efb7' else 'LICENSE.txt']
                for filename in names:
                    url = repo.replace('https://github.com/', 'https://raw.githubusercontent.com/') + '/' + commit + '/' + filename
                    record['notices'].append(notice(fetch(url), url, identity))
            for path in sorted((root / 'src').rglob('*.rs')):
                data = path.read_text(encoding='utf-8')
                for line in data.splitlines():
                    if re.match(r'\s*//\s*Copyright', line, re.I) and line.strip() not in record['copyrights']:
                        record['copyrights'].append(line.strip())

    output = Path(args.output)
    output.mkdir(parents=True, exist_ok=True)
    rust_parts = ['AccessKit UI dependencies. Pinned original license texts and attribution.\n',
                  'Platform names list Cargo-filtered source closures, including build-time crates.\n']
    for identity, record in sorted(components.items()):
        rust_parts.append('\n' + identity + '\nDeclared license: ' + record['declared_license'] +
                          '\nPlatforms: ' + ', '.join(sorted(set(record['platforms']))) + '\n' +
                          '\n'.join(record['copyrights']) + '\n')
    for sha, item in sorted(payloads.items()):
        rust_parts.append('\n' + '=' * 72 + '\nComponents: ' + ', '.join(sorted(item['components'])) +
                          '\nSource: ' + '\nSource: '.join(item['sources']) + '\nSHA-256: ' + sha + '\n\n' + item['text'])

    headers = []
    extra = ['Additional original notices in SDL3 and HarfBuzz source trees.\n',
             'This conservative collection also contains inactive platform/source portions.\n']
    seen = set()
    for family, source, revision in [('SDL3', Path(args.sdl_source), 'release-3.2.30'),
                                     ('HarfBuzz', Path(args.harfbuzz_source), '564bf9818a18709776856533829c0c04950773d6')]:
        candidates = sorted((source / 'src').rglob('*'))
        for path in candidates:
            if not path.is_file() or path.suffix not in ('.c', '.h', '.cpp', '.hh', '.inc'):
                continue
            text = path.read_text(encoding='utf-8', errors='strict')
            for match in re.finditer(r'/\*.*?\*/', text, re.S):
                block = match.group()
                if not re.search(r'copyright|permission to|public domain|licensed under', block, re.I):
                    continue
                sha = digest(block.encode())
                repo = 'libsdl-org/SDL' if family == 'SDL3' else 'harfbuzz/harfbuzz'
                origin = f'https://github.com/{repo}/blob/{revision}/{path.relative_to(source).as_posix()}'
                headers.append({'component': family, 'source': origin, 'sha256': sha})
                if sha not in seen:
                    seen.add(sha)
                    extra.append('\n' + '=' * 72 + '\nSource: ' + origin + '\nSHA-256: ' + sha + '\n\n' + block + '\n')
        if family == 'SDL3':
            for relative in ['src/video/yuv2rgb/LICENSE', 'src/hidapi/LICENSE-bsd.txt']:
                path = source / relative
                data = path.read_bytes(); sha = digest(data)
                origin = f'https://github.com/libsdl-org/SDL/blob/{revision}/{relative}'
                headers.append({'component': family, 'source': origin, 'sha256': sha})
                extra.append('\n' + '=' * 72 + '\nSource: ' + origin + '\nSHA-256: ' + sha + '\n\n' + data.decode())

    rust_text = ''.join(rust_parts).encode()
    extra_text = ''.join(extra).encode()
    (output / 'AccessKit-transitive.txt').write_bytes(rust_text)
    (output / 'UI-source-notices.txt').write_bytes(extra_text)
    manifest = {'format': 1, 'accesskit_original_lock_sha256': digest(lock_bytes),
                'open_audit_items': ['Rust standard-library/compiler runtime, including prebuilt compiler provenance',
                                     'System-provided dynamic runtime libraries and non-Cargo toolchain portions'],
                'components': [dict(record, platforms=sorted(set(record['platforms']))) for _, record in sorted(components.items())],
                'source_notices': headers,
                'bundles': {'AccessKit-transitive.txt': digest(rust_text), 'UI-source-notices.txt': digest(extra_text)}}
    Path(args.manifest).write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(f'{len(components)} Cargo components, {len(payloads)} distinct original payloads, {len(headers)} source notices.')
    print(f'Bundle bytes: {len(rust_text)}, {len(extra_text)}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--metadata', action='append', required=True, help='PLATFORM=metadata.json')
    parser.add_argument('--accesskit-source', required=True)
    parser.add_argument('--sdl-source', required=True)
    parser.add_argument('--harfbuzz-source', required=True)
    parser.add_argument('--output', required=True)
    parser.add_argument('--manifest', required=True)
    collect(parser.parse_args())
