"""Kill a real C worker at production callbacks; no product failure hooks."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import time


def wait_checkpoint(marker, proc, phase, timeout=30):
    deadline = time.monotonic() + timeout
    last_error = None
    while True:
        try:
            text = marker.read_text(encoding='utf-8')
        except (FileNotFoundError, PermissionError) as error:
            # CREATE_NEW exposes the name before the writer closes its handle.
            # Windows denies the read while that exclusive handle is open.
            last_error = error
        else:
            if text.endswith('\n'):
                fields = text.split()
                if len(fields) != 2 or not all(value.isdecimal() for value in fields) or int(fields[0]) != phase:
                    raise AssertionError('Unexpected checkpoint contents: ' + repr(text))
                return
        if proc.poll() is not None or time.monotonic() >= deadline:
            raise AssertionError(f'Checkpoint not reached: phase={phase}, exit={proc.poll()}, last_error={last_error}')
        time.sleep(0.02)


def main():
    worker, cli, test_root = map(Path, sys.argv[1:])
    worker, cli = worker.resolve(), cli.resolve()
    test_root.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix="Abbruch ü ", dir=test_root.resolve()))

    def command(*args):
        result = subprocess.run([str(cli), *map(str, args)], capture_output=True,
                                encoding="utf-8", timeout=30)
        if result.returncode:
            raise AssertionError(f"CLI failed {args}: {result.stderr}")
        return result.stdout

    def snapshot(path):
        return {str(p.relative_to(path)): hashlib.sha256(p.read_bytes()).hexdigest()
                for p in path.rglob("*") if p.is_file()}

    workspace = root / "Originale"
    command("new", workspace, "project", "Projekt ü")
    project = workspace / "project"
    (project / "attachment.bin").write_bytes(bytes(range(256)) * 4096)
    original = snapshot(project)
    archive = root / "valid.sbbackup"
    command("backup", workspace, "project", archive)
    archive_hash = hashlib.sha256(archive.read_bytes()).hexdigest()

    # Backup WRITE/RECHECK/VERIFY/PUBLISH, restore RESTORE/PUBLISH.
    for operation, phase in [("create", 1), ("create", 5), ("create", 2),
                             ("create", 4), ("restore", 3), ("restore", 4)]:
        case = root / f"{operation}-{phase}"
        case.mkdir()
        marker = case / "checkpoint.txt"
        if operation == "create":
            source, destination, identity = project, case / "new.sbbackup", "project"
            published = destination
        else:
            source, destination, identity = archive, case / "Ziel ü", "recovered"
            command("new", destination, "occupied", "Bestehendes Projekt")
            protected = snapshot(destination / "occupied")
            published = destination / identity
        proc = subprocess.Popen([str(worker), operation, str(source), str(destination),
                                 identity, str(phase), str(marker)], stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            wait_checkpoint(marker, proc, phase)
            assert proc.poll() is None, "worker must still be frozen"
            proc.kill()  # SIGKILL on POSIX; TerminateProcess on Windows.
            proc.communicate(timeout=10)
            assert proc.returncode != 0, "a hard kill must not report success"
        finally:
            if proc.poll() is None:
                proc.kill()
                proc.communicate(timeout=10)
        assert snapshot(project) == original
        assert hashlib.sha256(archive.read_bytes()).hexdigest() == archive_hash
        assert not published.exists(), "no partial archive/project may be published"
        stage_root = case if operation == "create" else destination
        stages = list(stage_root.glob(".sb-backup-*" if operation == "create" else ".sb-restore-*"))
        assert len(stages) == 1, stages
        remnants = snapshot(stages[0]) if stages[0].is_dir() else hashlib.sha256(stages[0].read_bytes()).hexdigest()
        if operation == "create":
            command("backup", workspace, "project", destination)
            command("inspect", destination)
            if phase == 4:
                command("inspect", stages[0])
                manual = case / "Manuell"
                manual.mkdir()
                command("restore", stages[0], manual, "from-complete-stage")
                assert {k: v for k, v in snapshot(manual / "from-complete-stage").items() if k != "brain.json"} == {k: v for k, v in original.items() if k != "brain.json"}
        else:
            assert snapshot(destination / "occupied") == protected
            assert "recovered" not in command("list", destination)
            command("restore", archive, destination, identity)
            recovered = snapshot(published)
            assert {k: v for k, v in recovered.items() if k != "brain.json"} == {
                k: v for k, v in original.items() if k != "brain.json"}
            assert json.loads((published / "brain.json").read_text())['id'] == identity
            assert snapshot(destination / "occupied") == protected
            assert command("list", destination).count("recovered") == 1
        after = snapshot(stages[0]) if stages[0].is_dir() else hashlib.sha256(stages[0].read_bytes()).hexdigest()
        assert after == remnants, "retry must preserve previous crash remnants"
        assert snapshot(project) == original
        print(f"Hard kill + verified retry: {operation}/{phase}")
    print(f"Six real process-kill checkpoints passed: {root}")


if __name__ == "__main__":
    main()
