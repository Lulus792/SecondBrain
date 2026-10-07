"""Check provenance against a compiler cache produced by real Cargo."""
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class CompilerTests(unittest.TestCase):
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
