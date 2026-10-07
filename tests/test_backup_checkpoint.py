"""Real delayed/locked marker publication, independent of the C backup worker."""
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from test_backup_crash import wait_checkpoint

WRITER = r'''
import ctypes, sys, time
path = sys.argv[1]
if sys.platform == 'win32':
    from ctypes import wintypes as w
    kernel = ctypes.WinDLL('kernel32', use_last_error=True)
    kernel.CreateFileW.argtypes = [w.LPCWSTR, w.DWORD, w.DWORD, w.LPVOID, w.DWORD, w.DWORD, w.HANDLE]
    kernel.CreateFileW.restype = w.HANDLE
    kernel.WriteFile.argtypes = [w.HANDLE, w.LPCVOID, w.DWORD, ctypes.POINTER(w.DWORD), w.LPVOID]
    kernel.WriteFile.restype = w.BOOL
    kernel.CloseHandle.argtypes = [w.HANDLE]
    kernel.CloseHandle.restype = w.BOOL
    handle = kernel.CreateFileW(path, 0x40000000, 0, None, 1, 0x80, None)
    if handle == ctypes.c_void_p(-1).value: raise ctypes.WinError(ctypes.get_last_error())
    written = w.DWORD()
    if not kernel.WriteFile(handle, b'4 ', 2, ctypes.byref(written), None): raise ctypes.WinError(ctypes.get_last_error())
    time.sleep(0.15)
    if not kernel.WriteFile(handle, b'65536\n', 6, ctypes.byref(written), None): raise ctypes.WinError(ctypes.get_last_error())
    kernel.CloseHandle(handle)
else:
    with open(path, 'wb') as marker:
        marker.write(b'4 '); marker.flush()
        time.sleep(0.15)
        marker.write(b'65536\n'); marker.flush()
sys.stdin.read(1)
'''


class CheckpointTests(unittest.TestCase):
    def test_partial_or_exclusively_locked_marker_is_not_ready(self):
        with tempfile.TemporaryDirectory(prefix='Checkpoint ü ') as temporary:
            marker = Path(temporary) / 'checkpoint.txt'
            proc = subprocess.Popen([sys.executable, '-c', WRITER, str(marker)], stdin=subprocess.PIPE,
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            try:
                wait_checkpoint(marker, proc, 4, timeout=5)
                self.assertEqual(marker.read_text(encoding='utf-8'), '4 65536\n')
                self.assertIsNone(proc.poll())
            finally:
                if proc.poll() is None:
                    proc.kill()
                proc.communicate(timeout=5)

    def test_wrong_phase_is_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            marker = Path(temporary) / 'checkpoint.txt'
            marker.write_text('3 65536\n', encoding='utf-8')
            with self.assertRaisesRegex(AssertionError, 'Unexpected checkpoint'):
                wait_checkpoint(marker, None, 4, timeout=1)

    def test_missing_marker_times_out(self):
        class Alive:
            def poll(self): return None
        with tempfile.TemporaryDirectory() as temporary:
            with self.assertRaisesRegex(AssertionError, 'Checkpoint not reached'):
                wait_checkpoint(Path(temporary) / 'missing', Alive(), 4, timeout=0.02)


if __name__ == '__main__':
    unittest.main()
