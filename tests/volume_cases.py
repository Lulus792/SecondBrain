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


def fill_volume(volume, capacity, remaining):
    filler = volume / 'fill.bin'
    with filler.open('wb', buffering=0) as stream:
        # Bounded independently of the mount: a missing ENOSPC fails the test.
        error = None
        # Large writes may be refused with room left for small files. Drain that
        # remainder too; ENOSPC for a 512 KiB request alone is insufficient.
        for chunk in (512 * 1024, 4096, 512, 1):
            while stream.tell() <= capacity + 512 * 1024:
                try:
                    written = stream.write(b'X' * chunk)
                    assert written and written > 0
                except OSError as failure:
                    assert failure.errno == errno.ENOSPC or getattr(failure, 'winerror', None) in (39, 112), failure
                    error = failure
                    break
            else:
                raise AssertionError('Bounded filesystem did not become full')
        assert error is not None, 'Bounded filesystem did not become full'
        assert error.errno == errno.ENOSPC or getattr(error, 'winerror', None) in (39, 112), error
        length = stream.seek(0, os.SEEK_END)
        assert length > 1024 * 1024
        # Leave only the space requested by this scenario.
        stream.truncate(length - remaining)
        os.fsync(stream.fileno())
    return filler, length, error


def exercise(worker, cli, root, volume, capacity, filesystem):
    """Caller must prove the mount identity and capacity before this call."""
    assert 0 < capacity <= 80 * 1024 * 1024
    command = lambda *args: successful([cli, *args])
    workspace = root / 'Original ü'
    command('new', workspace, 'project', 'Projekt ü')
    project = workspace / 'project'
    (project / 'attachment.bin').write_bytes(bytes(range(256)) * 4096)
    original = snapshot(project)
    archive = root / 'Sicherung ü.sbbackup'
    command('backup', workspace, 'project', archive)
    original_archive = hashlib.sha256(archive.read_bytes()).hexdigest()
    restore_root = volume / 'Wiederherstellung ü'
    command('new', restore_root, 'occupied', 'Bestehend')
    protected = snapshot(restore_root / 'occupied')
    phases = []
    for operation, phase in [('create-fill', 1), ('restore-fill', 3)]:
        filler, length, error = fill_volume(volume, capacity, 128 * 1024)
        marker = root / (operation + '.txt')
        source = project if phase == 1 else archive
        destination = volume / 'Fehlversuch ü.sbbackup' if phase == 1 else restore_root
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
    # Atomic note replacement/new creation and persisted settings use the same
    # real mount. Text bodies stay outside it so only the destination becomes full.
    notes_workspace = volume / 'Notizen ü'
    command('new', notes_workspace, 'notes', 'Notizen ü')
    notes_project = notes_workspace / 'notes'
    relative = 'knowledge/Erhalten ü.md'
    note = notes_project / relative
    note.write_bytes('# Bisherige Fassung ü\n\nMuss erhalten bleiben.\n'.encode())
    body = root / 'Entwurf ü.txt'
    desired = ('# Neue Fassung ü\n\n' + 'Längerer Text Ω und é.\n' * 65536).encode()
    body.write_bytes(desired)
    settings = volume / 'Einstellungen ü.conf'
    # A valid long remembered path makes the configuration require data blocks,
    # instead of potentially fitting inside filesystem metadata records.
    remembered_workspace = str(notes_workspace) + '/' + '/'.join(['Ω' * 60] * 25)
    assert len(remembered_workspace.encode()) < 4096
    successful([worker, 'settings-init', settings, remembered_workspace, 'notes', relative, '-'])
    assert settings.stat().st_size > 4096
    writes = []
    for mode, target, remaining in [('note-replace', note, 128 * 1024),
                                    ('note-new', notes_project / 'knowledge/Neu ü.md', 128 * 1024),
                                    ('settings-replace', settings, 0),
                                    ('settings-new', volume / 'Neue Einstellungen ü.conf', 0)]:
        before = snapshot(notes_project)
        old_settings = settings.read_bytes()
        exists = target.exists()
        old_target = target.read_bytes() if exists else None
        if mode.startswith('note'):
            args = [worker, 'note-save', notes_project, body, target.relative_to(notes_project).as_posix(), 'notes', '-']
        else:
            args = [worker, 'settings-update' if exists else 'settings-init', target, remembered_workspace, 'notes', relative, '-']
        filler, length, error = fill_volume(volume, capacity, remaining)
        try:
            result = execute(args)
            assert result.returncode == 1 and b'status=4:' in result.stderr, (mode, result.stderr)
            assert target.exists() == exists and (not exists or target.read_bytes() == old_target)
            assert snapshot(notes_project) == before and settings.read_bytes() == old_settings
            assert not list(notes_project.rglob('*.sbtmp-*')) and not list(volume.glob('*.tmp-*'))
            assert body.read_bytes() == desired, 'Caller-owned draft changed'
        finally:
            filler.unlink()
        successful(args)
        if mode.startswith('note'):
            assert target.read_bytes() == desired
        else:
            after = target.read_bytes()
            assert after.startswith(b'SecondBrain settings 2\n')
            if exists: assert after != old_target and b'font=150\n' in after
            else: assert b'font=100\n' in after
        assert body.read_bytes() == desired
        writes.append({'operation': mode, 'errno': error.errno, 'filled_bytes': length,
                       'remaining_requested': remaining, 'original_preserved': True,
                       'draft_unchanged': True, 'retry_succeeded': True})
        print(f'Real {filesystem} full-volume write protected existing/new target and retry: {mode}', flush=True)
    report = {'filesystem': filesystem, 'capacity': capacity, 'mount': str(volume),
              'phases': phases, 'writes': writes, 'detached': False,
              'limits': ['Disposable mounted image, not a physical power-loss test',
                         'Production C backup/restore; no error-injection API']}
    (root / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    return report
