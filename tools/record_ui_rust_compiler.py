#!/usr/bin/env python3
"""Record the compiler identified by Cargo's completed UI dependency build."""
import argparse
import hashlib
import json
from pathlib import Path
import re


def read_compiler(path):
    data = path.read_bytes()
    cache = json.loads(data)
    versions = {record['stdout'] for record in cache.get('outputs', {}).values()
                if record.get('success') is True and record.get('stdout', '').startswith('rustc ')}
    if len(versions) != 1:
        raise ValueError('Cargo cache does not identify exactly one successful compiler')
    detail = versions.pop()
    fields = {}
    for line in detail.splitlines()[1:]:
        if ': ' in line:
            key, value = line.split(': ', 1)
            fields[key] = value
    if not re.fullmatch('[0-9a-f]{40}', fields.get('commit-hash', '')) or not fields.get('release') or not fields.get('host'):
        raise ValueError('Cargo compiler identity lacks commit, release or host')
    return {'format': 1, 'cargo_cache_sha256': hashlib.sha256(data).hexdigest(),
            'compiler_commit': fields['commit-hash'], 'compiler_release': fields['release'],
            'compiler_host': fields['host'], 'version_output': detail}


def runtime_coverage(record, manifest_path, target):
    """Match the actual compiler and explicit build target to reviewed notices."""
    payload = manifest_path.read_bytes()
    manifest = json.loads(payload)
    if manifest.get('format') != 1 or manifest.get('scope') != 'conservative-supported-target-std-all-features':
        raise ValueError('Unknown Rust runtime notice manifest format or scope')
    matches = [compiler for compiler in manifest.get('compilers', [])
               if compiler.get('commit') == record['compiler_commit']
               and compiler.get('release') == record['compiler_release']]
    if len(matches) != 1:
        raise ValueError('Actual UI Rust compiler has no unique reviewed runtime source snapshot')
    compiler = matches[0]
    targets = [entry for entry in compiler.get('targets', []) if entry.get('target') == target]
    if len(targets) != 1:
        raise ValueError('UI build target is not covered by reviewed runtime sources: ' + target)
    bundle = manifest_path.parent / 'licenses/Rust-runtime.txt'
    if hashlib.sha256(bundle.read_bytes()).hexdigest() != manifest.get('bundle_sha256'):
        raise ValueError('Rust runtime notices do not match the reviewed manifest')
    return {'target': target, 'manifest_sha256': hashlib.sha256(payload).hexdigest(),
            'bundle_sha256': manifest['bundle_sha256'],
            'library_lock_sha256': compiler['library_lock_sha256'],
            'scope': manifest['scope']}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--target-dir', required=True)
    parser.add_argument('--output', required=True)
    parser.add_argument('--runtime-manifest', type=Path)
    parser.add_argument('--target', help='Actual Cargo build target; compiler host alone does not prove it')
    args = parser.parse_args()
    if bool(args.runtime_manifest) != bool(args.target):
        parser.error('--runtime-manifest and --target must be specified together')
    result = read_compiler(Path(args.target_dir) / '.rustc_info.json')
    if args.runtime_manifest:
        result['runtime_notice_coverage'] = runtime_coverage(result, args.runtime_manifest, args.target)
    path = Path(args.output)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    detail = result['version_output'].replace('%', '%25').replace('\r', '%0D').replace('\n', '%0A')
    print('::notice title=UI Rust compiler::' + detail)


if __name__ == '__main__':
    main()
