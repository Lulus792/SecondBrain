"""Check provenance against a compiler cache produced by real Cargo."""
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
import importlib.util

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('compiler_recorder', ROOT / 'tools/record_ui_rust_compiler.py')
recorder = importlib.util.module_from_spec(spec)
spec.loader.exec_module(recorder)


class CompilerTests(unittest.TestCase):
    def test_reviewed_compilers_cover_only_the_explicit_supported_targets(self):
        path = ROOT / 'third_party/rust-runtime-manifest.json'
        manifest = json.loads(path.read_text(encoding='utf-8'))
        for compiler in manifest['compilers']:
            record = {'compiler_commit': compiler['commit'], 'compiler_release': compiler['release']}
            for entry in compiler['targets']:
                coverage = recorder.runtime_coverage(record, path, entry['target'])
                self.assertEqual(coverage['target'], entry['target'])
                self.assertEqual(coverage['library_lock_sha256'], compiler['library_lock_sha256'])
            with self.assertRaises(ValueError):
                recorder.runtime_coverage(record, path, 'aarch64-unknown-linux-musl')

    def test_unknown_commit_or_release_is_rejected_even_with_supported_host(self):
        path = ROOT / 'third_party/rust-runtime-manifest.json'
        compiler = json.loads(path.read_text(encoding='utf-8'))['compilers'][0]
        for commit, release in [('0' * 40, compiler['release']), (compiler['commit'], '99.0.0')]:
            with self.subTest(commit=commit, release=release), self.assertRaises(ValueError):
                recorder.runtime_coverage({'compiler_commit': commit, 'compiler_release': release,
                                           'compiler_host': 'x86_64-unknown-linux-gnu'},
                                          path, 'x86_64-unknown-linux-gnu')

    def test_changed_notice_bundle_is_rejected(self):
        original = ROOT / 'third_party/rust-runtime-manifest.json'
        compiler = json.loads(original.read_text(encoding='utf-8'))['compilers'][0]
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            shutil.copyfile(original, root / original.name)
            (root / 'licenses').mkdir()
            (root / 'licenses/Rust-runtime.txt').write_text('incomplete notices', encoding='utf-8')
            with self.assertRaises(ValueError):
                recorder.runtime_coverage({'compiler_commit': compiler['commit'],
                                           'compiler_release': compiler['release']},
                                          root / original.name, 'x86_64-unknown-linux-gnu')

    @unittest.skipUnless(shutil.which('cargo') and shutil.which('rustc'), 'Cargo/Rust compiler unavailable')
    def test_actual_cargo_build_identifies_compiler(self):
        with tempfile.TemporaryDirectory(prefix='SecondBrain Rust ü ') as temporary:
            root = Path(temporary)
            (root / 'src').mkdir()
            (root / 'Cargo.toml').write_text('[package]\nname="sb-provenance-fixture"\nversion="0.0.0"\nedition="2021"\n', encoding='utf-8')
            (root / 'src/lib.rs').write_text('#![no_std]\npub fn value() -> u8 { 1 }\n', encoding='utf-8')
            build = subprocess.run([shutil.which('cargo'), 'build', '--offline', '--target-dir', str(root / 'target')], cwd=root,
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            self.assertEqual(build.returncode, 0, build.stderr.decode(errors='replace'))
            output = root / 'compiler.json'
            result = subprocess.run([sys.executable, str(ROOT / 'tools/record_ui_rust_compiler.py'),
                                     '--target-dir', str(root / 'target'), '--output', str(output)],
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            self.assertEqual(result.returncode, 0, result.stderr.decode(errors='replace'))
            record = json.loads(output.read_text(encoding='utf-8'))
            actual = subprocess.check_output([shutil.which('rustc'), '-vV'], cwd=root).decode('utf-8')
            self.assertEqual(record['version_output'], actual)
            self.assertIn(b'::notice title=UI Rust compiler::', result.stdout)
            manifest_path = ROOT / 'third_party/rust-runtime-manifest.json'
            manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
            covered = any(c['commit'] == record['compiler_commit']
                          and c['release'] == record['compiler_release']
                          and any(t['target'] == record['compiler_host'] for t in c['targets'])
                          for c in manifest['compilers'])
            if covered:
                checked = subprocess.run([sys.executable, str(ROOT / 'tools/record_ui_rust_compiler.py'),
                                          '--target-dir', str(root / 'target'), '--output', str(output),
                                          '--runtime-manifest', str(manifest_path),
                                          '--target', record['compiler_host']],
                                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                self.assertEqual(checked.returncode, 0, checked.stderr.decode(errors='replace'))
                verified = json.loads(output.read_text(encoding='utf-8'))
                self.assertEqual(verified['runtime_notice_coverage']['target'], record['compiler_host'])

    def test_cli_rejects_unknown_compiler_before_writing_a_coverage_record(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            detail = ('rustc 99.0.0 (fixture)\ncommit-hash: ' + '0' * 40
                      + '\nrelease: 99.0.0\nhost: x86_64-unknown-linux-gnu\n')
            (root / '.rustc_info.json').write_text(json.dumps({'outputs': {
                'fixture': {'success': True, 'stdout': detail}}}), encoding='utf-8')
            output = root / 'coverage.json'
            result = subprocess.run([sys.executable, str(ROOT / 'tools/record_ui_rust_compiler.py'),
                                     '--target-dir', str(root), '--output', str(output),
                                     '--runtime-manifest', str(ROOT / 'third_party/rust-runtime-manifest.json'),
                                     '--target', 'x86_64-unknown-linux-gnu'],
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn(b'no unique reviewed runtime source snapshot', result.stderr)
            self.assertFalse(output.exists())

    def test_unidentified_compiler_is_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / '.rustc_info.json').write_text('{"outputs":{}}', encoding='utf-8')
            result = subprocess.run([sys.executable, str(ROOT / 'tools/record_ui_rust_compiler.py'),
                                     '--target-dir', str(root), '--output', str(root / 'output.json')],
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            self.assertNotEqual(result.returncode, 0)
            self.assertFalse((root / 'output.json').exists())


if __name__ == '__main__':
    unittest.main()
