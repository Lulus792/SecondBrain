"""Run the actual application with its normal renderer and isolated project data."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile


def hashes(root):
    return {p.relative_to(root).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in root.rglob('*') if p.is_file()}


def verify(app, cli, output):
    output.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='Normaler Start ü ', dir=output.resolve()))
    workspace = root / 'Wissen ü'
    subprocess.run([str(cli), 'new', str(workspace), 'probe', 'Laufzeitprüfung'],
                   capture_output=True, check=True, timeout=30)
    note = workspace / 'probe/knowledge/Eigene Notiz.md'
    note.write_bytes('# Unverändert\r\n\r\nΩ, e\u0301 und 🧭.\r\n'.encode('utf-8'))
    before = hashes(workspace)
    report = root / 'Laufzeit ü.json'
    def invoke(*args):
        return subprocess.run([str(app), *map(str, args)], capture_output=True, timeout=60)
    result = invoke('--workspace', workspace, '--runtime-probe', report)
    if result.returncode != 0:
        raise RuntimeError('Normal-renderer probe failed: ' + result.stderr.decode('utf-8', errors='replace'))
    data = json.loads(report.read_bytes())
    version = invoke('--version')
    if (data['format'] != 1 or data['build'] != version.stdout.decode('utf-8')
            or data['rendered_frames'] != 4 or data['window_visible'] is not True or not data['renderer'] or not data['video_driver']
            or not data['loaded_modules'] or any(not isinstance(x, str) or not x for x in data['loaded_modules'])
            or data['scope'] != 'normal-renderer-start-after-four-frames'):
        raise RuntimeError('Incomplete runtime probe report')
    original = report.read_bytes()
    retry = invoke('--workspace', workspace, '--runtime-probe', report)
    if retry.returncode != 1 or report.read_bytes() != original:
        raise RuntimeError('Runtime probe replaced an existing report')
    rejected = root / 'Must not exist.json'
    for args in [('--runtime-probe', rejected),
                 ('--workspace', workspace, '--runtime-probe', rejected, '--settings', root / 'Must not exist.conf'),
                 ('--workspace', workspace, '--runtime-probe', rejected, '--snapshot', root / 'Must not exist.bmp')]:
        if invoke(*args).returncode != 2 or rejected.exists():
            raise RuntimeError('Ambiguous probe arguments were accepted')
    if hashes(workspace) != before or (root / 'Must not exist.conf').exists() or (root / 'Must not exist.bmp').exists():
        raise RuntimeError('Runtime probe changed isolated project or settings files')
    data.update(application_sha256=hashlib.sha256(app.read_bytes()).hexdigest(),
                project_files_unchanged=len(before), existing_report_preserved=True,
                ambiguous_arguments_rejected=True)
    (root / 'verification.json').write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print('Normal renderer:', data['renderer'], '; video:', data['video_driver'],
          '; observed modules:', len(data['loaded_modules']), '; preserved project files:', len(before))
    print('Evidence:', root)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--app', required=True, type=Path)
    parser.add_argument('--cli', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    verify(args.app.resolve(), args.cli.resolve(), args.output)
