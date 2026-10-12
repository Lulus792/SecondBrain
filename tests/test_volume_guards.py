"""Reject wrong mounted identities/capacities before any filler writes."""
import ctypes
import json
import os
from pathlib import Path
from types import SimpleNamespace
import tempfile
import unittest
from unittest.mock import patch
import test_backup_volume_native as native


class VolumeGuards(unittest.TestCase):
    def linux_case(self, fault):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); volume = root / 'Volume'; image = root / 'limited.ext4'
            real_stat = Path.stat
            commands = []
            entered = False
            def stat(path, *args, **kwargs):
                if path == volume:
                    return SimpleNamespace(st_dev=real_stat(root).st_dev + 1)
                return real_stat(path, *args, **kwargs)
            def command(args):
                args = list(map(str, args)); commands.append(args)
                if '--find' in args:
                    return b'/dev/loop123\n'
                if 'BACK-FILE' in args:
                    return str(root / ('foreign.img' if fault == 'identity' else image.name)).encode() + b'\n'
                if 'findmnt' in args:
                    return b'xfs\n' if fault == 'type' else b'ext4\n'
                return b''
            # The foreign file exists so samefile must reject its identity.
            (root / 'foreign.img').write_bytes(b'foreign')
            with patch.object(native, 'successful', side_effect=command), \
                 patch.object(native.os, 'geteuid', return_value=0, create=True), \
                 patch.object(native.os, 'getuid', return_value=1000, create=True), \
                 patch.object(native.os, 'getgid', return_value=1000, create=True), \
                 patch.object(native.os.path, 'ismount', return_value=True), \
                 patch.object(Path, 'stat', stat), \
                 patch.object(native.os, 'statvfs', return_value=SimpleNamespace(f_blocks=1000000 if fault == 'capacity' else 8192, f_frsize=4096), create=True):
                with self.assertRaises((AssertionError, RuntimeError)):
                    with native.linux_volume(root):
                        entered = True
                        raise RuntimeError('body failed')
            self.assertEqual(entered, fault == 'body')
            self.assertTrue(any('umount' in c for c in commands))
            self.assertTrue(any('--detach' in c and '/dev/loop123' in c for c in commands))
            self.assertFalse((volume / 'fill.bin').exists())

    def test_linux_foreign_image(self): self.linux_case('identity')
    def test_linux_wrong_filesystem(self): self.linux_case('type')
    def test_linux_unbounded_mount(self): self.linux_case('capacity')
    def test_linux_body_failure_detaches(self): self.linux_case('body')

    def windows_case(self, fault):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); image = root / 'limited.vhd'
            commands = []
            mounted = [False]
            entered = False
            kernel = SimpleNamespace(GetLogicalDrives=lambda: 1)
            def command(args):
                commands.append(list(map(str, args)))
                script = Path(args[-1]).read_text(encoding='utf-16')
                self.assertNotIn('select disk ', script.lower())
                self.assertNotIn('\nclean', script.lower())
                if 'create vdisk' in script:
                    image.write_bytes(b'owned fixture')
                    mounted[0] = True
                if 'detach vdisk' in script:
                    mounted[0] = False
                return b''
            info = {'DriveLetter': 'C' if fault == 'identity' else 'Z',
                    'FileSystem': 'NTFS', 'FileSystemLabel': 'SecondBrainVolumeTest',
                    'Size': 512 * 1024 * 1024 if fault == 'capacity' else 64 * 1024 * 1024}
            # Mock a callable API object so ctypes' restype assignment remains real.
            class Drives:
                def __call__(self): return 1
            kernel.GetLogicalDrives = Drives()
            real_exists = Path.exists
            def exists(path):
                if str(path) == 'Z:\\': return mounted[0]
                return real_exists(path)
            with patch.object(ctypes, 'WinDLL', return_value=kernel, create=True), \
                 patch.object(native, 'successful', side_effect=command), \
                 patch.object(native.subprocess, 'run', return_value=SimpleNamespace(returncode=0, stdout=json.dumps(info).encode(), stderr=b'')), \
                 patch.object(native.time, 'sleep'), patch.object(Path, 'exists', exists):
                with self.assertRaises((AssertionError, RuntimeError)):
                    with native.windows_volume(root):
                        entered = True
                        raise RuntimeError('body failed')
            self.assertEqual(entered, fault == 'body')
            self.assertEqual(len(commands), 2)
            self.assertIn('detach.txt', commands[-1][-1])
            self.assertFalse((root / 'fill.bin').exists())

    def test_windows_foreign_volume(self): self.windows_case('identity')
    def test_windows_unbounded_volume(self): self.windows_case('capacity')
    def test_windows_body_failure_detaches(self): self.windows_case('body')


if __name__ == '__main__':
    unittest.main()
