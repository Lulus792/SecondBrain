#!/usr/bin/env python3
"""Collect original notices for reviewed Rust library source snapshots.

Developer tool (Python 3.11+), never run by the application. The conservative
target-filtered std closure includes build dependencies and all Cargo features.
"""
import argparse
import hashlib
import io
import json
import lzma
from pathlib import Path, PurePosixPath
import re
import ssl
import tarfile
import urllib.request

REVIEWED = {
    '48a229ceaefd4985c50990b14116b6d856af0985': ('1.98.1', 'd1c5dbdf53bfebd7de60f26a171819db3b28ebfd75f3fa99c3286893a5a7b7a6'),
    'b940084d7eb6a299eb4bfeb8e34901bc051e7ac4': ('1.99.0', '53d9ccff0c906de434b65af90d5c30063894b940fc839261f1fb06ef2e69e3d3'),
}
RUST_ORIGINALS = ['LICENSE-MIT', 'LICENSE-APACHE', 'COPYRIGHT',
                  'library/compiler-builtins/LICENSE.txt',
                  'library/stdarch/LICENSE-MIT', 'library/stdarch/LICENSE-APACHE']
ORIGINAL_HASHES = {
    'LICENSE-MIT': 'b71bd43a069ca0641a9ecfe585ca7b3c53b5cc1608f8b68321168698e28b5ea1',
    'LICENSE-APACHE': '62c7a1e35f56406896d7aa7ca52d0cc0d272ac022b5d2796e7d6905db8a3636a',
    'COPYRIGHT': '172020dbfd5b53a226dfde77616190a48dcff519b0bc0e6deb91a8450782c4af',
    'library/compiler-builtins/LICENSE.txt': 'ab6eec6caf0fa5775e411c7a8bc6a45c4ef2956b0980b157ab74fc5cd62a928b',
    'library/stdarch/LICENSE-MIT': '29662666b44dff84977b46e05642cdef910bc3a93a17b5fd86e632bafa59cf21',
    'library/stdarch/LICENSE-APACHE': 'a60eea817514531668d7e00765731449fe14d059d3249e0bc93b36de45f759f2',
}
TARGETS = {'x86_64-apple-darwin', 'aarch64-apple-darwin', 'x86_64-pc-windows-msvc', 'x86_64-unknown-linux-gnu'}
SOURCE_ARCHIVES = {
    '48a229ceaefd4985c50990b14116b6d856af0985': '5c846ebcebcc7e2e0777a4cdaa12051691593f16a7e94edbae5e6241cc62d98c',
    'b940084d7eb6a299eb4bfeb8e34901bc051e7ac4': '3f1f9b7ed48f4596fc87889b7b3c61747336a55c9c22db1ab0c697e0aadb77aa',
}


def target_closure(data):
    packages = {p['id']: p for p in data['packages']}
    nodes = {n['id']: n for n in data['resolve']['nodes']}
    roots = [p['id'] for p in packages.values() if p['name'] == 'std' and p['version'] == '0.0.0']
    if len(roots) != 1 or not {'backtrace', 'panic-unwind'}.issubset(nodes[roots[0]]['features']):
        raise ValueError('Missing conservative std feature closure')
    pending, seen = roots[:], set()
    while pending:
        key = pending.pop()
        if key in seen:
            continue
        seen.add(key)
        pending.extend(d['pkg'] for d in nodes[key]['deps'] if any(k['kind'] != 'dev' for k in d['dep_kinds']))
    return [packages[key] for key in sorted(seen)], nodes[roots[0]]['features']


def sha(data):
    return hashlib.sha256(data).hexdigest()


def fetch(url, cache):
    path = cache / sha(url.encode('utf-8'))
    if path.is_file():
        return path.read_bytes()
    context = ssl.create_default_context(cafile='/etc/ssl/cert.pem' if Path('/etc/ssl/cert.pem').is_file() else None)
    with urllib.request.urlopen(url, context=context, timeout=30) as response:
        data = response.read()
    path.write_bytes(data)
    return data


def registry_original(package, cache):
    import tomllib
    name, version = package['name'], package['version']
    if not re.fullmatch('[a-zA-Z0-9_-]+', name) or not re.fullmatch('[a-zA-Z0-9.+_-]+', version):
        raise ValueError('Unrecognized crate identity')
    url = f'https://static.crates.io/crates/{name}/{name}-{version}.crate'
    data = fetch(url, cache)
    if sha(data) != package['checksum']:
        raise ValueError('Changed original crate archive: ' + name + '@' + version)
    prefix = name + '-' + version + '/'
    originals = []
    with tarfile.open(fileobj=io.BytesIO(data), mode='r:gz') as archive:
        manifest = archive.extractfile(prefix + 'Cargo.toml')
        if manifest is None:
            raise ValueError('Original crate manifest missing')
        crate = tomllib.loads(manifest.read().decode('utf-8'))['package']
        declared = crate.get('license') or crate.get('license-file')
        if not isinstance(declared, str) or not declared:
            raise ValueError('Original license assignment missing: ' + name)
        for member in sorted(archive.getmembers(), key=lambda item: item.name):
            if not member.isfile() or not member.name.startswith(prefix):
                continue
            relative = member.name[len(prefix):]
            path = PurePosixPath(relative)
            if '..' in path.parts or path.is_absolute():
                raise ValueError('Unsafe original archive member')
            if re.match(r'(?i)^(LICENSE|COPYING|NOTICE|COPYRIGHT|AUTHORS)', path.name):
                stream = archive.extractfile(member)
                payload = stream.read()
                payload.decode('utf-8')
                originals.append((relative, payload, url + ' : ' + relative))
        if not originals:
            raise ValueError('Missing original texts require manual source review: ' + name)
    return declared, originals, url


def collect(args):
    import tomllib
    cache = Path(args.cache)
    cache.mkdir(parents=True, exist_ok=True)
    components, compilers, payloads, roots, source_notices = {}, [], {}, [], []
    source_archives = dict(specification.split('=', 1) for specification in args.rust_source_archive)
    metadata = {}
    for specification in args.metadata:
        identity, filename = specification.split('=', 1)
        commit, target = identity.split(':', 1)
        if commit not in REVIEWED or target not in TARGETS or (commit, target) in metadata:
            raise ValueError('Unknown or duplicate target metadata')
        raw = Path(filename).read_bytes()
        closure, features = target_closure(json.loads(raw))
        metadata[(commit, target)] = (closure, features, sha(raw))

    def add_original(owner, data, source):
        text = data.decode('utf-8')
        if '\x00' in text:
            raise ValueError('Invalid original text')
        key = sha(data)
        item = payloads.setdefault(key, {'text': text, 'owners': set(), 'sources': set()})
        item['owners'].add(owner)
        item['sources'].add(source)
        return {'sha256': key, 'bytes': len(data), 'source': source}

    seen = set()
    for specification in args.compiler_source:
        commit, directory = specification.split('=', 1)
        if commit not in REVIEWED or commit in seen:
            raise ValueError('Unknown or duplicate compiler source snapshot')
        seen.add(commit)
        release, expected_lock = REVIEWED[commit]
        lock = (Path(directory) / 'library/Cargo.lock').read_bytes()
        if sha(lock) != expected_lock:
            raise ValueError('Changed original Rust library lock')
        packages = tomllib.loads(lock.decode('utf-8'))['package']
        locked = {(p['name'], p['version']): p for p in packages}
        targets = []
        selected, selected_metadata = {}, {}
        for target in sorted(TARGETS):
            if (commit, target) not in metadata:
                raise ValueError('Missing supported target metadata')
            closure, features, metadata_hash = metadata[(commit, target)]
            identities = []
            for package in closure:
                identity = (package['name'], package['version'])
                if identity not in locked:
                    raise ValueError('Metadata package not in original lock')
                selected[identity] = locked[identity]
                selected_metadata[identity] = package
                if package.get('source'):
                    identities.append(package['name'] + '@' + package['version'])
            targets.append({'target': target, 'metadata_sha256': metadata_hash,
                            'std_features': features, 'registry_components': sorted(identities)})
        registry = [p for p in selected.values() if p.get('source')]
        compilers.append({'commit': commit, 'release': release, 'library_lock_sha256': sha(lock),
                          'registry_packages': len(registry),
                          'full_library_lock_registry_packages': sum(bool(p.get('source')) for p in packages),
                          'targets': targets,
                          'in_tree_packages': [{'name': p['name'], 'version': p['version'],
                                                'declared_license': selected_metadata[(p['name'], p['version'])].get('license')}
                                               for p in selected.values() if not p.get('source')]})
        for relative in RUST_ORIGINALS:
            url = f'https://raw.githubusercontent.com/rust-lang/rust/{commit}/{relative}'
            original = fetch(url, cache)
            if sha(original) != ORIGINAL_HASHES[relative]:
                raise ValueError('Changed reviewed in-tree original: ' + relative)
            roots.append(dict(add_original('Rust source ' + release, original, url),
                              compiler_commit=commit, file=relative))
        for package in registry:
            if package['source'] != 'registry+https://github.com/rust-lang/crates.io-index':
                raise ValueError('Unreviewed registry origin')
            identity = package['name'] + '@' + package['version']
            if identity not in components:
                declared, originals, url = registry_original(package, cache)
                components[identity] = {'name': package['name'], 'version': package['version'],
                                        'declared_license': declared, 'archive_source': url,
                                        'archive_sha256': package['checksum'], 'compiler_commits': [],
                                        'notices': [add_original(identity, data, source) for _, data, source in originals]}
            elif components[identity]['archive_sha256'] != package['checksum']:
                raise ValueError('Conflicting original crate archive')
            components[identity]['compiler_commits'].append(commit)
        archive_path = Path(source_archives[commit])
        archive_bytes = archive_path.read_bytes()
        if sha(archive_bytes) != SOURCE_ARCHIVES[commit]:
            raise ValueError('Changed original Rust source archive')
        prefix = f'rust-src-{release}/rust-src/lib/rustlib/src/rust/'
        archive_url = f'https://static.rust-lang.org/dist/rust-src-{release}.tar.xz'
        vendors = {p['name'] + '-' + p['version'] for p in registry}
        # Source paths are visited in deterministic order. Decompress once:
        # seeking backwards through an XZ stream would re-read the whole archive.
        with tarfile.open(fileobj=io.BytesIO(lzma.decompress(archive_bytes)), mode='r:') as archive:
            archived_lock = archive.extractfile(prefix + 'library/Cargo.lock')
            if archived_lock is None or sha(archived_lock.read()) != expected_lock:
                raise ValueError('Source archive and original compiler lock disagree')
            for member in sorted(archive.getmembers(), key=lambda item: item.name):
                if not member.isfile() or not member.name.startswith(prefix):
                    continue
                relative = member.name[len(prefix):]
                path = PurePosixPath(relative)
                if '..' in path.parts or path.is_absolute():
                    raise ValueError('Unsafe Rust source archive member')
                if relative.startswith('library/vendor/') and (len(path.parts) < 3 or path.parts[2] not in vendors):
                    continue
                origin = archive_url + ' : ' + relative
                if relative == 'src/llvm-project/libunwind/LICENSE.TXT':
                    data = archive.extractfile(member).read()
                    source_notices.append(dict(add_original('Rust source ' + release, data, origin),
                                               compiler_commit=commit, file=relative))
                if path.suffix not in {'.rs', '.c', '.h', '.cpp', '.S', '.s', '.inc'}:
                    continue
                text = archive.extractfile(member).read().decode('utf-8')
                pattern = r'/\*.*?\*/|(?:^[ \t]*//[^\n]*(?:\n|$))+'
                if path.suffix in {'.S', '.s'}:
                    pattern += r'|(?:^[ \t]*[#;][^\n]*(?:\n|$))+'
                comments = re.finditer(pattern, text, re.S | re.M)
                for comment in comments:
                    block = comment.group()
                    if not re.search(r'copyright|permission to|public domain|licensed under|SPDX-License-Identifier', block, re.I):
                        continue
                    source_notices.append(dict(add_original('Rust source ' + release, block.encode('utf-8'), origin),
                                               compiler_commit=commit, file=relative))
        compilers[-1]['source_archive'] = archive_url
        compilers[-1]['source_archive_sha256'] = sha(archive_bytes)

    parts = ['Original notices for reviewed Rust standard-library source snapshots.\n',
             'Conservative std closure covers four supported targets, all Cargo features and build dependencies.\n',
             'This file does not assert that every listed component is linked into every application.\n']
    for compiler in compilers:
        parts.append('\nRust source ' + compiler['release'] + '\nCommit: ' + compiler['commit'] +
                     '\nLibrary lock SHA-256: ' + compiler['library_lock_sha256'] + '\n')
    for identity, item in sorted(components.items()):
        parts.append('\n' + identity + '\nDeclared license: ' + item['declared_license'] +
                     '\nCompiler snapshots: ' + ', '.join(item['compiler_commits']) + '\n')
    for key, item in sorted(payloads.items()):
        parts.append('\n' + '=' * 72 + '\nOwners: ' + ', '.join(sorted(item['owners'])) +
                     '\nSource: ' + '\nSource: '.join(sorted(item['sources'])) + '\nSHA-256: ' + key +
                     '\nBytes: ' + str(len(item['text'].encode('utf-8'))) + '\n\n' + item['text'])
    bundle = ''.join(parts).encode('utf-8')
    Path(args.output).write_bytes(bundle)
    manifest = {'format': 1, 'scope': 'conservative-supported-target-std-all-features',
                'compilers': compilers, 'components': [item for _, item in sorted(components.items())],
                'in_tree_originals': roots, 'source_notices': source_notices, 'bundle_sha256': sha(bundle),
                'open_audit_items': ['Full in-tree attribution and target-specific linked inventory',
                                     'Actual Windows/Linux compiler coverage and LLVM/system unwinders']}
    Path(args.manifest).write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
    print(len(compilers), 'compiler snapshots;', len(components), 'registry identities;',
          len(payloads), 'original payloads;', len(bundle), 'bundle bytes')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--compiler-source', action='append', required=True, help='COMMIT=original-source-directory')
    parser.add_argument('--metadata', action='append', required=True, help='COMMIT:TARGET=target-filtered-all-features-metadata.json')
    parser.add_argument('--rust-source-archive', action='append', required=True, help='COMMIT=verified-rust-src-release.tar.xz')
    parser.add_argument('--cache', required=True)
    parser.add_argument('--output', required=True)
    parser.add_argument('--manifest', required=True)
    collect(parser.parse_args())
