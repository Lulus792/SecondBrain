"""Real ENOSPC on a bounded disposable HFS+ image, never on the host volume."""
import json
import os
from pathlib import Path
import plistlib
import subprocess
import sys
import tempfile
from volume_cases import exercise, successful


def main():
    worker, cli, base = map(Path, sys.argv[1:])
    worker, cli = worker.resolve(), cli.resolve()
    assert sys.platform == 'darwin'
    base.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='Speicher ü ', dir=base.resolve()))
    image, volume = root / 'limited.dmg', root / 'Volume'
    volume.mkdir()
    successful(['hdiutil', 'create', '-size', '32m', '-fs', 'HFS+', '-volname',
                'SecondBrainTest', '-nospotlight', image])
    attached = False
    try:
        output = successful(['hdiutil', 'attach', image, '-mountpoint', volume,
                             '-nobrowse', '-noautoopen', '-plist'])
        attached = True
        entities = plistlib.loads(output)['system-entities']
        assert any(e.get('mount-point') and os.path.samefile(e['mount-point'], volume) for e in entities)
        assert os.path.ismount(volume) and volume.stat().st_dev != root.stat().st_dev
        space = os.statvfs(volume); capacity = space.f_blocks * space.f_frsize
        assert 0 < capacity <= 40 * 1024 * 1024
        report = exercise(worker, cli, root, volume, capacity, 'HFS+')
    finally:
        if attached:
            result = subprocess.run(['hdiutil', 'detach', str(volume)], capture_output=True, timeout=60)
            if result.returncode:
                result = subprocess.run(['hdiutil', 'detach', '-force', str(volume)], capture_output=True, timeout=60)
            assert result.returncode == 0, f'Owned test volume could not be detached: {volume}'
    report['detached'] = True
    (root / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(f'Disposable 32 MiB image detached: {root}', flush=True)


if __name__ == '__main__':
    main()
