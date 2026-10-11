"""Preserve the native settings byte contract under Windows text-mode rules."""
import importlib.util
from pathlib import Path, PureWindowsPath
import tempfile
import unittest
from unittest.mock import patch
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('update_verifier', ROOT / 'tools/verify_package_update.py')
verifier = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verifier)


class UpdateFixtureTests(unittest.TestCase):
    def test_settings_keep_lf_and_unicode_paths_under_windows_newline_translation(self):
        original_open = Path.open

        def windows_open(path, mode='r', *args, **kwargs):
            if 'w' in mode and 'b' not in mode:
                kwargs['newline'] = '\r\n'
            return original_open(path, mode, *args, **kwargs)

        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / 'settings.conf'
            for workspace in (PureWindowsPath('D:/Arbeit ü/100%'), Path('/tmp/Arbeit ü/100%')):
                with self.subTest(workspace=workspace), patch.object(Path, 'open', windows_open):
                    control = Path(temporary) / 'text-mode-control.txt'
                    control.write_text('header\n', encoding='utf-8')
                    self.assertEqual(control.read_bytes(), b'header\r\n')
                    verifier.write_fixture_settings(output, workspace)
                    raw = output.read_bytes()
                    self.assertTrue(raw.startswith(b'SecondBrain settings 2\n'))
                    self.assertNotIn(b'\r', raw)
                    self.assertEqual(raw.count(b'\n'), 12)
                    values = dict(line.split('=', 1) for line in raw.decode('utf-8').splitlines()[1:])
                    self.assertEqual(unquote(values['workspace']), str(workspace))
                    self.assertEqual(unquote(values['note']), 'knowledge/Handschrift ü.md')
                    self.assertEqual(values['project'], 'projekt')


if __name__ == '__main__':
    unittest.main()
