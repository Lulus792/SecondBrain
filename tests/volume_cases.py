"""Production backup operations on a previously verified, bounded test volume."""
import errno
import hashlib
import json
import os
from pathlib import Path
import subprocess


def execute(args):
    return subprocess.run(list(map(str, args)), capture_output=True, timeout=60)


def successful(args):
    result = execute(args)
    assert result.returncode == 0, (args, result.stdout, result.stderr)
    return result.stdout


def snapshot(path):
    return {p.relative_to(path).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in path.rglob('*') if p.is_file()}


def exercise(worker, cli, root, volume, capacity, filesystem):
    """Caller must prove the mount identity and capacity before this call."""
    assert 0 < capacity <= 80 * 1024 * 1024
    command = lambda *args: successful([cli, *args])
    workspace = root / 'Original'
    command('new', workspace, 'project', 'Projekt ü')
    project = workspace / 'project'
    (project / 'attachment.bin').write_bytes(bytes(range(256)) * 4096)
    original = snapshot(project)
    archive = root / 'valid.sbbackup'
    command('backup', workspace, 'project', archive)
    original_archive = hashlib.sha256(archive.read_bytes()).hexdigest()
    restore_root = volume / 'Wiederherstellung ü'
    command('new', restore_root, 'occupied', 'Bestehend')
    protected = snapshot(restore_root / 'occupied')
    phases = []
    for operation, phase in [('create-fill', 1), ('restore-fill', 3)]:
        filler = volume / 'fill.bin'
        with filler.open('wb', buffering=0) as stream:
            # Bounded independently of the mount: a missing ENOSPC fails the test.
            error = None
            while stream.tell() <= capacity + 512 * 1024:
                try:
                    written = stream.write(b'X' * (512 * 1024))
                    assert written and written > 0
                except OSError as failure:
                    error = failure
                    break
            assert error is not None, 'Bounded filesystem did not become full'
            assert error.errno == errno.ENOSPC or getattr(error, 'winerror', None) in (39, 112), error
            length = stream.seek(0, os.SEEK_END)
            assert length > 1024 * 1024
            # Permit real partial progress, but not the complete 1 MiB attachment.
            stream.truncate(length - 128 * 1024)
            os.fsync(stream.fileno())
        marker = root / (operation + '.txt')
        source = project if phase == 1 else archive
        destination = volume / 'failed.sbbackup' if phase == 1 else restore_root
        final = destination if phase == 1 else destination / 'recovered'
        result = execute([worker, operation, source, destination,
                          'project' if phase == 1 else 'recovered', phase, marker])
        assert result.returncode == 1 and b'status=4:' in result.stderr, result.stderr
        assert marker.exists() and int(marker.read_text().split()[1]) >= 65536, 'No confirmed partial progress'
        assert not final.exists()
        assert snapshot(project) == original
        assert snapshot(restore_root / 'occupied') == protected
        assert hashlib.sha256(archive.read_bytes()).hexdigest() == original_archive
        assert not list(volume.glob('.sb-backup-*')) and not list(restore_root.glob('.sb-restore-*'))
        filler.unlink()
        if phase == 1:
            command('backup', workspace, 'project', destination)
            command('inspect', destination)
        else:
            command('restore', archive, restore_root, 'recovered')
            restored = snapshot(final)
            assert {k: v for k, v in restored.items() if k != 'brain.json'} == {k: v for k, v in original.items() if k != 'brain.json'}
        assert snapshot(project) == original
        assert snapshot(restore_root / 'occupied') == protected
        assert hashlib.sha256(archive.read_bytes()).hexdigest() == original_archive
        phases.append({'operation': operation, 'filled_bytes': length,
                       'errno': error.errno, 'winerror': getattr(error, 'winerror', None),
                       'progress_bytes': int(marker.read_text().split()[1]),
                       'originals_unchanged': True, 'retry_succeeded': True})
        print(f'Real {filesystem} full-volume failure after partial progress and successful retry: {operation}', flush=True)
    report = {'filesystem': filesystem, 'capacity': capacity, 'mount': str(volume),
              'phases': phases, 'detached': False,
              'limits': ['Disposable mounted image, not a physical power-loss test',
                         'Production C backup/restore; no error-injection API']}
    (root / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    return report
