"""Original payload integrity and supported-target Rust source coverage."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('runtime_collector', ROOT / 'tools/collect_rust_runtime_notices.py')
collector = importlib.util.module_from_spec(spec)
spec.loader.exec_module(collector)


def validate(data, bundle):
    if data.get('format') != 1 or data.get('scope') != 'conservative-supported-target-std-all-features':
        raise ValueError('Unknown inventory scope')
    if hashlib.sha256(bundle).hexdigest() != data['bundle_sha256']:
        raise ValueError('Changed bundle')
    identities = {(c['name'], c['version']) for c in data['components']}
    if len(identities) != 20 or len(identities) != len(data['components']):
        raise ValueError('Changed or duplicated registry identities')
    commits = {c['commit'] for c in data['compilers']}
    if commits != set(collector.REVIEWED) or len(data['compilers']) != 2:
        raise ValueError('Changed compiler snapshots')
    for compiler in data['compilers']:
        commit = compiler['commit']
        if (compiler['release'], compiler['library_lock_sha256']) != collector.REVIEWED[commit]:
            raise ValueError('Changed original source lock')
        if compiler['source_archive_sha256'] != collector.SOURCE_ARCHIVES[commit]:
            raise ValueError('Changed original source archive')
        if {t['target'] for t in compiler['targets']} != collector.TARGETS or len(compiler['targets']) != 4:
            raise ValueError('Missing supported target')
        for target in compiler['targets']:
            expected = 6 if target['target'] == 'x86_64-pc-windows-msvc' else 13
            if len(target['registry_components']) != expected or not {'backtrace', 'panic-unwind'}.issubset(target['std_features']):
                raise ValueError('Changed conservative closure')
            for identity in target['registry_components']:
                if not any(c['name'] + '@' + c['version'] == identity and commit in c['compiler_commits'] for c in data['components']):
                    raise ValueError('Unrepresented target component')
        builtins = next(p for p in compiler['in_tree_packages'] if p['name'] == 'compiler_builtins')
        if builtins['declared_license'] != 'MIT AND Apache-2.0 WITH LLVM-exception AND (MIT OR Apache-2.0)':
            raise ValueError('Lost compiler-builtins AND condition')
        if {p['file'] for p in data['in_tree_originals'] if p['compiler_commit'] == commit} != set(collector.RUST_ORIGINALS):
            raise ValueError('Missing in-tree original')
        if any(p['sha256'] != collector.ORIGINAL_HASHES[p['file']] for p in data['in_tree_originals'] if p['compiler_commit'] == commit):
            raise ValueError('Changed reviewed in-tree original')
    records = data['in_tree_originals'] + data['source_notices']
    for component in data['components']:
        if not component['declared_license'] or not component['notices']:
            raise ValueError('Unassigned registry original')
        records += component['notices']
    for record in records:
        marker = ('\nSHA-256: ' + record['sha256'] + '\nBytes: ' + str(record['bytes']) + '\n\n').encode()
        try:
            start = bundle.index(marker) + len(marker)
        except ValueError:
            raise ValueError('Missing original payload') from None
        payload = bundle[start:start + record['bytes']]
        if hashlib.sha256(payload).hexdigest() != record['sha256']:
            raise ValueError('Changed original payload')
        if not record['source'].startswith('https://'):
            raise ValueError('Missing original origin')
    if not any(p['file'] == 'src/llvm-project/libunwind/LICENSE.TXT' for p in data['source_notices']):
        raise ValueError('LLVM unwinder original missing')
    bundle.decode('utf-8')
    if b'\x00' in bundle:
        raise ValueError('Invalid literal data')


class RuntimeInventoryTests(unittest.TestCase):
    def setUp(self):
        self.data = json.loads((ROOT / 'third_party/rust-runtime-manifest.json').read_text(encoding='utf-8'))
        self.bundle = (ROOT / 'third_party/licenses/Rust-runtime.txt').read_bytes()

    def test_supported_source_snapshots_and_exact_original_payloads(self):
        validate(self.data, self.bundle)

    def test_removed_target_rejected(self):
        data = copy.deepcopy(self.data)
        data['compilers'][0]['targets'].pop()
        with self.assertRaises(ValueError): validate(data, self.bundle)

    def test_changed_lock_rejected(self):
        data = copy.deepcopy(self.data)
        data['compilers'][0]['library_lock_sha256'] = '0' * 64
        with self.assertRaises(ValueError): validate(data, self.bundle)

    def test_changed_payload_rejected_even_after_bundle_rehash(self):
        record = self.data['in_tree_originals'][0]
        marker = ('\nSHA-256: ' + record['sha256'] + '\nBytes: ' + str(record['bytes']) + '\n\n').encode()
        start = self.bundle.index(marker) + len(marker)
        broken = self.bundle[:start] + b'!' + self.bundle[start + 1:]
        data = copy.deepcopy(self.data)
        data['bundle_sha256'] = hashlib.sha256(broken).hexdigest()
        with self.assertRaisesRegex(ValueError, 'Changed original payload'): validate(data, broken)

    def test_and_condition_not_reduced_to_mit(self):
        data = copy.deepcopy(self.data)
        next(p for p in data['compilers'][0]['in_tree_packages'] if p['name'] == 'compiler_builtins')['declared_license'] = 'MIT'
        with self.assertRaises(ValueError): validate(data, self.bundle)

    def test_missing_backtrace_feature_is_not_a_supported_superset(self):
        data = {'packages': [{'id': 'std', 'name': 'std', 'version': '0.0.0'}],
                'resolve': {'nodes': [{'id': 'std', 'features': ['panic-unwind'], 'deps': []}]}}
        with self.assertRaises(ValueError): collector.target_closure(data)

    def test_closure_omits_dev_dependencies_and_handles_cycles(self):
        packages = [{'id': name, 'name': name, 'version': '0.0.0'} for name in ['std', 'normal', 'dev']]
        data = {'packages': packages, 'resolve': {'nodes': [
            {'id': 'std', 'features': ['backtrace', 'panic-unwind'], 'deps': [
                {'pkg': 'normal', 'dep_kinds': [{'kind': None}]}, {'pkg': 'dev', 'dep_kinds': [{'kind': 'dev'}]}]},
            {'id': 'normal', 'features': [], 'deps': [{'pkg': 'std', 'dep_kinds': [{'kind': None}]}]},
            {'id': 'dev', 'features': [], 'deps': []}]}}
        closure, _ = collector.target_closure(data)
        self.assertEqual({p['name'] for p in closure}, {'std', 'normal'})


if __name__ == '__main__':
    unittest.main()
