import importlib.util
from pathlib import Path
import plistlib
import subprocess
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('signing', Path(__file__).resolve().parents[1]/'tools/sign_package.py')
signing = importlib.util.module_from_spec(spec)
spec.loader.exec_module(signing)


class SigningTests(unittest.TestCase):
    def fixture(self, directory):
        root = directory/'source ü'
        (root/'secondbrain.app/Contents/MacOS').mkdir(parents=True)
        (root/'licenses').mkdir()
        for name in ('LICENSE', 'QUICKSTART.txt', 'secondbrain-cli', 'secondbrain.app/Contents/MacOS/secondbrain'):
            (root/name).write_bytes(b'fixture bytes')
        (root/'secondbrain.app/Contents/Info.plist').write_bytes(
            plistlib.dumps({'CFBundleIdentifier': 'org.secondbrain.desktop'}))
        return root

    def test_ad_hoc_cannot_claim_publisher_identity(self):
        with self.assertRaises(ValueError):
            signing.settings('macos', 'Developer ID: example', 'ABCDEFGHIJ', None, None, True)
        with self.assertRaises(ValueError):
            signing.settings('macos', '-', 'ABCDEFGHIJ', None, None, False)

    def test_windows_requires_explicit_certificate_and_timestamp(self):
        for certificate, url in ((None, 'https://timestamp.example'), ('a'*40, None),
                                 ('bad', 'https://timestamp.example'), ('a'*40, 'https://user:secret@timestamp.example')):
            with self.assertRaises(ValueError):
                signing.settings('windows', None, None, certificate, url, False)

    def test_existing_output_and_nested_output_preserve_source(self):
        with tempfile.TemporaryDirectory() as temporary, patch.object(signing.sys, 'platform', 'darwin'):
            root = self.fixture(Path(temporary))
            before = signing.snapshot(root)
            output = Path(temporary)/'existing'
            output.mkdir()
            (output/'keep').write_bytes(b'keep')
            for target in (output, root/'nested'):
                with self.assertRaises(ValueError):
                    signing.sign(root, target, 'macos', adhoc=True)
            self.assertEqual(signing.snapshot(root), before)
            self.assertEqual((output/'keep').read_bytes(), b'keep')
            self.assertFalse((root/'nested').exists())

    def test_unknown_package_entry_is_not_copied(self):
        with tempfile.TemporaryDirectory() as temporary, patch.object(signing.sys, 'platform', 'darwin'):
            root = self.fixture(Path(temporary))
            (root/'private-notes.md').write_bytes(b'not a distribution file')
            output = Path(temporary)/'output'
            with self.assertRaisesRegex(ValueError, 'known distribution entries'):
                signing.sign(root, output, 'macos', adhoc=True)
            self.assertFalse(output.exists())

    def test_tool_failure_preserves_input_and_marks_output_failed(self):
        with tempfile.TemporaryDirectory() as temporary, patch.object(signing.sys, 'platform', 'darwin'), \
             patch.object(signing, 'native_tool', return_value='codesign'):
            root = self.fixture(Path(temporary)); before = signing.snapshot(root)
            output = Path(temporary)/'output'
            def fail(args):
                raise RuntimeError('signing tool failure fixture')
            with self.assertRaises(RuntimeError):
                signing.sign(root, output, 'macos', adhoc=True, runner=fail)
            self.assertEqual(signing.snapshot(root), before)
            self.assertTrue((output/'FAILED.txt').is_file())
            self.assertFalse((output/'signing.json').exists())

    def test_signing_must_not_change_package_data(self):
        with tempfile.TemporaryDirectory() as temporary, patch.object(signing.sys, 'platform', 'darwin'), \
             patch.object(signing, 'native_tool', return_value='codesign'):
            root = self.fixture(Path(temporary)); before = signing.snapshot(root)
            output = Path(temporary)/'output'
            def corrupt(args):
                (output/'SecondBrain/QUICKSTART.txt').write_bytes(b'corrupted')
                return subprocess.CompletedProcess(args, 0, b'', b'CodeDirectory flags=0x10000(runtime)\n')
            with self.assertRaisesRegex(RuntimeError, 'non-program'):
                signing.sign(root, output, 'macos', adhoc=True, runner=corrupt)
            self.assertEqual(signing.snapshot(root), before)
            self.assertFalse((output/'signing.json').exists())


if __name__ == '__main__':
    unittest.main()
