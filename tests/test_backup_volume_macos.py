"""Real ENOSPC on a bounded disposable HFS+ image, never on the host volume."""
import errno
import hashlib
import os
from pathlib import Path
import plistlib
import subprocess
import sys
import tempfile

worker, cli, base = map(Path, sys.argv[1:])
worker, cli = worker.resolve(), cli.resolve()
assert sys.platform == "darwin"
base.mkdir(parents=True, exist_ok=True)
root = Path(tempfile.mkdtemp(prefix="Speicher ü ", dir=base.resolve()))
image, volume = root / "limited.dmg", root / "Volume"
volume.mkdir()

def execute(args):
    return subprocess.run(list(map(str, args)), capture_output=True, timeout=60)

def successful(args):
    result = execute(args)
    assert result.returncode == 0, result.stderr.decode("utf-8", errors="replace")
    return result.stdout

def command(*args):
    return successful([cli, *args])

def snapshot(path):
    return {str(p.relative_to(path)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in path.rglob("*") if p.is_file()}

successful(["hdiutil", "create", "-size", "32m", "-fs", "HFS+", "-volname", "SecondBrainTest", "-nospotlight", image])
attached = False
try:
    output = successful(["hdiutil", "attach", image, "-mountpoint", volume, "-nobrowse", "-noautoopen", "-plist"])
    attached = True
    entities = plistlib.loads(output)["system-entities"]
    assert any(e.get("mount-point") and os.path.samefile(e["mount-point"], volume) for e in entities)
    assert os.path.ismount(volume) and volume.stat().st_dev != root.stat().st_dev
    space = os.statvfs(volume)
    assert 0 < space.f_blocks * space.f_frsize <= 40 * 1024 * 1024
    workspace = root / "Original"
    command("new", workspace, "project", "Projekt ü")
    project = workspace / "project"
    (project / "attachment.bin").write_bytes(bytes(range(256)) * 4096)
    original = snapshot(project)
    archive = root / "valid.sbbackup"
    command("backup", workspace, "project", archive)
    original_archive = hashlib.sha256(archive.read_bytes()).hexdigest()
    restore_root = volume / "Wiederherstellung"
    command("new", restore_root, "occupied", "Bestehend")
    protected = snapshot(restore_root / "occupied")
    for operation, phase in [("create-fill", 1), ("restore-fill", 3)]:
        filler = volume / "fill.bin"
        with filler.open("wb", buffering=0) as stream:
            try:
                while True:
                    stream.write(b"X" * (512 * 1024))
                # unreachable: the image must actually report ENOSPC
            except OSError as failure:
                assert failure.errno == errno.ENOSPC, failure
            # Leave room for a partial file, but less than the 1 MiB attachment.
            length = stream.seek(0, os.SEEK_END)
            assert length > 1024 * 1024
            stream.truncate(length - 128 * 1024)
            os.fsync(stream.fileno())
        marker = root / (operation + ".txt")
        source = project if phase == 1 else archive
        destination = volume / "failed.sbbackup" if phase == 1 else restore_root
        final = destination if phase == 1 else destination / "recovered"
        result = execute([worker, operation, source, destination, "project" if phase == 1 else "recovered", phase, marker])
        assert result.returncode == 1 and b"status=4:" in result.stderr, result.stderr
        assert marker.exists() and int(marker.read_text().split()[1]) >= 65536, "must fail after real partial progress"
        assert not final.exists()
        assert snapshot(project) == original
        assert snapshot(restore_root / "occupied") == protected
        assert hashlib.sha256(archive.read_bytes()).hexdigest() == original_archive
        assert not list(volume.glob(".sb-backup-*")) and not list(restore_root.glob(".sb-restore-*"))
        filler.unlink()
        if phase == 1:
            command("backup", workspace, "project", destination)
            command("inspect", destination)
        else:
            command("restore", archive, restore_root, "recovered")
            restored = snapshot(final)
            assert {k: v for k, v in restored.items() if k != "brain.json"} == {k: v for k, v in original.items() if k != "brain.json"}
        print(f"Real HFS+ ENOSPC after partial progress and successful retry: {operation}")
finally:
    if attached:
        result = execute(["hdiutil", "detach", volume])
        if result.returncode:
            result = execute(["hdiutil", "detach", "-force", volume])
        assert result.returncode == 0, f"Owned test volume could not be detached: {volume}"
print(f"Disposable 32 MiB image detached: {root}")
