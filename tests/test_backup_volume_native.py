"""Opt-in Linux/ext4 and Windows/NTFS tests on newly created bounded images."""
from contextlib import contextmanager
import ctypes
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import time
from volume_cases import exercise, successful


@contextmanager
def linux_volume(root):
    image, volume = root / 'limited.ext4', root / 'Volume'
    volume.mkdir()
    with image.open('xb') as stream:
        stream.truncate(32 * 1024 * 1024)
    successful(['mkfs.ext4', '-q', '-F', '-m', '0', image])
    elevated = [] if os.geteuid() == 0 else ['sudo', '-n']
    loop = successful([*elevated, 'losetup', '--find', '--show', image]).decode().strip()
    assert re.fullmatch(r'/dev/loop[0-9]+', loop), loop
    mounted = False
    try:
        successful([*elevated, 'mount', '-t', 'ext4', '-o', 'nosuid,nodev', loop, volume])
        mounted = True
        assert os.path.ismount(volume) and volume.stat().st_dev != root.stat().st_dev
        backing = successful([*elevated, 'losetup', '--noheadings', '--output', 'BACK-FILE', loop]).decode().strip()
        assert os.path.samefile(backing, image), (backing, image)
        kind = successful(['findmnt', '--noheadings', '--output', 'FSTYPE', '--target', volume]).decode().strip()
        assert kind == 'ext4', kind
        space = os.statvfs(volume); capacity = space.f_blocks * space.f_frsize
        assert 0 < capacity <= 40 * 1024 * 1024
        successful([*elevated, 'chown', f'{os.getuid()}:{os.getgid()}', volume])
        yield volume, capacity, 'ext4'
    finally:
        try:
            if mounted:
                successful([*elevated, 'umount', volume])
        finally:
            successful([*elevated, 'losetup', '--detach', loop])


@contextmanager
def windows_volume(root):
    kernel = ctypes.WinDLL('kernel32', use_last_error=True)
    kernel.GetLogicalDrives.restype = ctypes.c_uint32
    mask = kernel.GetLogicalDrives()
    assert mask, ctypes.get_last_error()
    letter = next((chr(n + ord('A')) for n in range(25, 17, -1) if not mask & (1 << n)), None)
    assert letter, 'No unused test drive letter'
    volume = Path(letter + ':\\')
    assert not volume.exists()
    image = root / 'limited.vhd'
    assert not image.exists()
    assert not any(c in str(image) for c in '"\r\n'), 'Invalid DiskPart path'
    # DiskPart focuses only this new file. Never select/clean a numbered disk.
    script = root / 'attach.txt'
    script.write_text(f'create vdisk file="{image}" maximum=64 type=fixed\n'
                      'attach vdisk\ncreate partition primary\n'
                      'format fs=ntfs label=SecondBrainVolumeTest quick\n'
                      f'assign letter={letter}\nexit\n', encoding='utf-16')
    started = False
    try:
        started = True
        successful(['diskpart.exe', '/s', script])
        # Independently map the created image to the assigned volume via Storage.
        env = dict(os.environ, SB_TEST_VOLUME_IMAGE=str(image))
        query = '$ErrorActionPreference="Stop"; Get-DiskImage -ImagePath $env:SB_TEST_VOLUME_IMAGE | Get-Disk | Get-Partition | Get-Volume | Select-Object DriveLetter,FileSystem,FileSystemLabel,Size | ConvertTo-Json -Compress'
        result = subprocess.run(['powershell.exe', '-NoProfile', '-NonInteractive', '-Command', query], env=env, capture_output=True, timeout=60)
        assert result.returncode == 0, result.stderr
        info = json.loads(result.stdout.decode('utf-8-sig'))
        assert isinstance(info, dict) and info['DriveLetter'] == letter and info['FileSystem'] == 'NTFS', info
        assert info['FileSystemLabel'] == 'SecondBrainVolumeTest' and 0 < info['Size'] <= 80 * 1024 * 1024, info
        assert volume.exists()
        yield volume, int(info['Size']), 'NTFS'
    finally:
        if started and image.exists():
            # Microsoft requires a gap between consecutive DiskPart invocations.
            time.sleep(15)
            detach = root / 'detach.txt'
            detach.write_text(f'select vdisk file="{image}"\ndetach vdisk\nexit\n', encoding='utf-16')
            successful(['diskpart.exe', '/s', detach])
            assert not volume.exists(), 'Owned test image remained attached'


def main():
    worker, cli, base = map(Path, sys.argv[1:])
    worker, cli = worker.resolve(), cli.resolve()
    assert sys.platform.startswith('linux') or sys.platform == 'win32'
    base.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='volume-', dir=base.resolve()))
    backend = windows_volume if sys.platform == 'win32' else linux_volume
    with backend(root) as (volume, capacity, filesystem):
        report = exercise(worker, cli, root, volume, capacity, filesystem)
    report['detached'] = True
    (root / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(f'Owned bounded image detached; evidence: {root}', flush=True)


if __name__ == '__main__':
    main()
