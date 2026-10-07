"""Exercise entry diagnostics with actual native CMake/CTest processes."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cmake') and shutil.which('ctest'), 'CMake and CTest required')
class EntryTests(unittest.TestCase):
    def run_entry(self, directory, log):
        return subprocess.run([sys.executable, str(ROOT / 'tools/test_entry.py'),
                               '--test-dir', str(directory), '--config', 'Release',
                               '--log', str(log)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    def check_case(self, test, expected_success):
        with tempfile.TemporaryDirectory(prefix='SecondBrain Unicode ü ') as temporary:
            root = Path(temporary)
            directory = root / 'test dir'
            directory.mkdir()
            (directory / 'CTestTestfile.cmake').write_text(test, encoding='utf-8')
            log = root / 'logs' / 'entry.log'
            result = self.run_entry(directory, log)
            self.assertEqual(result.returncode == 0, expected_success, result.stdout.decode(errors='replace'))
            detail = log.read_text(encoding='utf-8', errors='replace')
            self.assertIn('Checked test entry', detail)
            self.assertIn(str(directory), detail)
            self.assertIn('CMake exit status:', detail)
            self.assertTrue((directory / 'checked-ctest.log').is_file(), result.stdout.decode(errors='replace'))
            self.assertEqual(result.stdout, log.read_bytes())

    def test_success_with_spaces_and_unicode(self):
        cmake = Path(shutil.which('cmake')).as_posix()
        self.check_case('add_test(pass "' + cmake + '" -E true)\n', True)

    def test_failed_ctest_is_not_hidden_by_logging(self):
        cmake = Path(shutil.which('cmake')).as_posix()
        self.check_case('add_test(fail "' + cmake + '" -E false)\n', False)

    def test_empty_test_set_is_an_error(self):
        self.check_case('', False)

    def test_missing_test_directory_keeps_entry_diagnostic(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            log = root / 'entry.log'
            result = self.run_entry(root / 'missing', log)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('Checked test entry', log.read_text(encoding='utf-8', errors='replace'))
            self.assertIn('CMake exit status:', log.read_text(encoding='utf-8', errors='replace'))

    def test_error_before_long_success_tail_remains_in_ci_annotation(self):
        with tempfile.TemporaryDirectory() as temporary:
            log = Path(temporary) / 'entry.log'
            log.write_text('ValueError: original license hash mismatch\n' + 'Later successful test\n' * 400,
                           encoding='utf-8')
            result = subprocess.run([shutil.which('cmake'), '-DSB_DIAGNOSTIC_LOG=' + str(log),
                                     '-P', str(ROOT / 'tools/ci_log_report.cmake')],
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            self.assertEqual(result.returncode, 0)
            self.assertIn(b'ValueError: original license hash mismatch', result.stdout)


if __name__ == '__main__':
    unittest.main()
