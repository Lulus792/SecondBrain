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


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--target-dir', required=True)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()
    result = read_compiler(Path(args.target_dir) / '.rustc_info.json')
    path = Path(args.output)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    detail = result['version_output'].replace('%', '%25').replace('\r', '%0D').replace('\n', '%0A')
    print('::notice title=UI Rust compiler::' + detail)


if __name__ == '__main__':
    main()
