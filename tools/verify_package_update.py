#!/usr/bin/env python3
"""Exercise two extracted packages against isolated persistent project data."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
from urllib.parse import quote


def hashes(root):
    return {p.relative_to(root).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in sorted(root.rglob('*')) if p.is_file()}


def programs(root):
    if sys.platform == 'darwin':
        return root / 'secondbrain.app/Contents/MacOS/secondbrain', root / 'secondbrain-cli'
    suffix = '.exe' if sys.platform == 'win32' else ''
    return root / ('secondbrain' + suffix), root / ('secondbrain-cli' + suffix)


def invoke(program, *args, expected=0):
    result = subprocess.run([str(program), *map(str, args)], capture_output=True, timeout=90)
    if result.returncode != expected:
        raise RuntimeError(str(program) + ' returned ' + str(result.returncode) + ': '
                           + result.stderr.decode('utf-8', errors='replace'))
    return result


def write_fixture_settings(settings, workspace):
    # The native settings format uses LF bytes on every platform. Text-mode
    # output would translate them to CRLF on Windows and invalidate the fixture.
    settings.write_bytes(('SecondBrain settings 2\nworkspace=' + quote(str(workspace), safe='/:-._~')
                         + '\nproject=projekt\nnote=' + quote('knowledge/Handschrift ü.md', safe='/:-._~')
                         + '\nfont=150\nwidth=1000\nheight=700\ndark=1\nsolid=1\nmotion=1\nfollow-theme=0\ncontrast=0\n').encode('utf-8'))


def verify(before_root, after_root, evidence):
    old_app, old_cli = programs(before_root)
    new_app, new_cli = programs(after_root)
    versions = {}
    for label, app, cli in [('before', old_app, old_cli), ('after', new_app, new_cli)]:
        app_info = invoke(app, '--version')
        cli_info = invoke(cli, '--version')
        if app_info.stdout != cli_info.stdout or app_info.stderr or cli_info.stderr:
            raise RuntimeError('App and CLI build identities differ: ' + label)
        versions[label] = app_info.stdout.decode('utf-8')

    workspace = evidence / 'Arbeitsordner ü'
    invoke(old_cli, 'new', workspace, 'projekt', 'Projekt mit eigenen Notizen')
    project = workspace / 'projekt'
    metadata = project / 'brain.json'
    original = metadata.read_bytes()
    # Preserve an intentionally noncanonical unknown field, not just parsed values.
    metadata.write_bytes(original.rstrip()[:-1] + b',\n  "custom" : {"keep": [1, true, null]}\n}\n')
    note = project / 'knowledge/Handschrift ü.md'
    note.write_bytes('# Eigene Erkenntnis ü\r\n\r\nUpdate-Prüfwort: Ω, e\u0301 und 🧭.\r\n'.encode('utf-8'))
    (project / 'inbox/roh.md').write_bytes(b'# Eingang\n\nUnveraenderter Rohtext.\n')
    (project / 'archive/alt.md').write_bytes(b'# Archiv\n\nAuch beim Update erhalten.\n')
    (project / 'attachment.bin').write_bytes(bytes(range(256)))
    legacy = workspace / 'legacy'
    legacy.mkdir()
    (legacy / 'brain.json').write_bytes(b'{ "name" : "Altformat", "custom" : 42 }\n')
    (legacy / 'START.md').write_bytes(b'# Altformat\n\nKeine automatische Migration.\n')
    future = workspace / 'future'
    future.mkdir()
    (future / 'brain.json').write_bytes(b'{"name":"Zukunft","schema_version":2}\n')
    (future / 'START.md').write_bytes(b'# Unbekanntes Schema\n')
    baseline = hashes(workspace)

    def unchanged(stage):
        if hashes(workspace) != baseline:
            raise RuntimeError('Project bytes changed during ' + stage)

    def read_project(cli):
        listing = invoke(cli, 'list', workspace, expected=1)
        if b'projekt\t' not in listing.stdout or b'legacy\t' not in listing.stdout or b'future:' not in listing.stderr:
            raise RuntimeError('Valid and unsupported projects were not distinguished')
        context = invoke(cli, 'context', workspace, 'projekt').stdout
        search = invoke(cli, 'search', workspace, 'projekt', 'Update-Prüfwort').stdout
        if 'Handschrift ü.md'.encode('utf-8') not in search:
            raise RuntimeError('Existing Unicode note not searchable')
        invoke(cli, 'context', workspace, 'future', expected=1)
        invoke(cli, 'new', workspace, 'projekt', 'Darf nichts ersetzen', expected=1)
        unchanged('CLI reading and rejected replacement')
        return context, search

    old_read = read_project(old_cli)
    settings = evidence / 'settings.conf'
    write_fixture_settings(settings, workspace)
    invoke(old_app, '--settings', settings, '--snapshot', evidence / 'before.bmp')
    unchanged('old app opening')
    saved_settings = settings.read_bytes()
    if b'project=projekt\n' not in saved_settings or b'note=knowledge/Handschrift%20%C3%BC.md\n' not in saved_settings:
        raise RuntimeError('Old app did not restore the selected project and note')
    invoke(new_app, '--settings', settings, '--snapshot', evidence / 'after.bmp')
    unchanged('new app opening without workspace arguments')
    if settings.read_bytes() != saved_settings:
        raise RuntimeError('New app lost stored navigation or display settings')
    if read_project(new_cli) != old_read:
        raise RuntimeError('Context or search output changed during update')

    archive = evidence / 'Sicherung nach Update.sbbackup'
    restored = evidence / 'Wiederhergestellt ü'
    restored.mkdir()
    invoke(new_cli, 'backup', workspace, 'projekt', archive)
    invoke(new_cli, 'inspect', archive)
    invoke(new_cli, 'restore', archive, restored, 'kopie')
    source_files = hashes(project)
    restored_files = hashes(restored / 'kopie')
    source_files.pop('brain.json')
    restored_files.pop('brain.json')
    if source_files != restored_files:
        raise RuntimeError('Backup after update failed byte-for-byte restoration')
    restored_metadata = json.loads((restored / 'kopie/brain.json').read_bytes())
    if restored_metadata['id'] != 'kopie' or restored_metadata['custom'] != {'keep': [1, True, None]}:
        raise RuntimeError('Restore lost project identity or custom metadata')
    unchanged('backup and restoration')
    invoke(old_app, '--settings', settings, '--snapshot', evidence / 'rollback.bmp')
    unchanged('rollback opening')
    if settings.read_bytes() != saved_settings:
        raise RuntimeError('Old app cannot reuse update settings')
    for name in ('before.bmp', 'after.bmp', 'rollback.bmp'):
        if (evidence / name).stat().st_size <= 54:
            raise RuntimeError('Missing rendered app snapshot: ' + name)
    return {'versions': versions, 'distinct_builds': versions['before'] != versions['after'],
            'program_hashes': {label: hashlib.sha256(path.read_bytes()).hexdigest()
                               for label, path in [('before_app', old_app), ('before_cli', old_cli),
                                                   ('after_app', new_app), ('after_cli', new_cli)]},
            'workspace_files': baseline, 'project_bytes_unchanged': True,
            'settings_unchanged': True, 'restored_content_matches': True,
            'limitations': ['Snapshot processes do not prove interactive editing or screenreader use',
                            'Manual switch between extracted packages; no installer or power-failure simulation']}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--before-root', required=True, type=Path)
    parser.add_argument('--after-root', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='Update ü ', dir=args.output.resolve()))
    report = verify(args.before_root.resolve(), args.after_root.resolve(), evidence)
    (evidence / 'report.json').write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print('Package update preserved', len(report['workspace_files']), 'files; distinct builds:', report['distinct_builds'])
    print('Evidence:', evidence)


if __name__ == '__main__':
    main()
