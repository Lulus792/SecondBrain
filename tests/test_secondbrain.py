import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

from secondbrain import create_brain


class CreateBrainTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def test_creation_links_repository_with_spaces_and_unicode(self):
        repository = self.root / "Projekt ü (1)"
        repository.mkdir()
        destination = self.root / "brains" / "test"
        create_brain("test", "Mein Projekt", destination, repository)
        metadata = json.loads((destination / "brain.json").read_text(encoding="utf-8"))
        linked = destination / metadata["project_root"]
        self.assertEqual(linked.resolve(), repository.resolve())
        project = (destination / "PROJECT.md").read_text(encoding="utf-8")
        self.assertIn("Projekt%20%C3%BC%20%281%29", project)
        self.assertEqual(metadata["template_version"], 1)
        self.assertTrue((destination / "knowledge" / "NOTE_TEMPLATE.md").is_file())

    def test_existing_directory_is_preserved_even_when_empty(self):
        destination = self.root / "existing"
        destination.mkdir()
        with self.assertRaises(FileExistsError):
            create_brain("test", "Test", destination)
        self.assertEqual(list(destination.iterdir()), [])
        sentinel = destination / "STATE.md"
        sentinel.write_text("Nutzerwissen", encoding="utf-8")
        with self.assertRaises(FileExistsError):
            create_brain("test", "Test", destination)
        self.assertEqual(sentinel.read_text(encoding="utf-8"), "Nutzerwissen")

    def test_existing_file_is_preserved(self):
        destination = self.root / "file"
        destination.write_text("Bestehend", encoding="utf-8")
        with self.assertRaises(FileExistsError):
            create_brain("test", "Test", destination)
        self.assertEqual(destination.read_text(encoding="utf-8"), "Bestehend")
    def test_dangling_symlink_is_preserved(self):
        symlink = self.root / "link"
        try:
            symlink.symlink_to(self.root / "missing")
        except OSError as error:
            self.skipTest(f"Betriebssystem erlaubt keine Symlinks: {error}")
        with self.assertRaises(FileExistsError):
            create_brain("test", "Test", symlink)
        self.assertTrue(symlink.is_symlink())

    def test_invalid_ids_never_create_files(self):
        for project_id in ("../outside", "/tmp/outside", "Test", "", "a/b", "a--b",
                           "con", "nul", "com1", "lpt9", "a" * 65):
            with self.subTest(project_id=project_id):
                with self.assertRaises(ValueError):
                    create_brain(project_id, "Test", self.root / "new")
        self.assertEqual(list(self.root.iterdir()), [])

    def test_missing_repository_and_invalid_name_leave_no_destination(self):
        destination = self.root / "new"
        with self.assertRaises(ValueError):
            create_brain("test", "Test", destination, self.root / "missing")
        for name in ("", "   ", "Name\nAnweisung"):
            with self.assertRaises(ValueError):
                create_brain("test", name, destination)
        self.assertFalse(destination.exists())

    def test_brains_do_not_share_mutable_state(self):
        first, second = self.root / "first", self.root / "second"
        create_brain("first", "Erstes", first)
        create_brain("second", "Zweites", second)
        (first / "STATE.md").write_text("Neue Erkenntnis", encoding="utf-8")
        self.assertNotIn("Neue Erkenntnis", (second / "STATE.md").read_text(encoding="utf-8"))
        self.assertIn("Zweites", (second / "START.md").read_text(encoding="utf-8"))

    def test_different_drive_falls_back_to_absolute_uri(self):
        repository = self.root / "Quellen ü"
        repository.mkdir()
        destination = self.root / "brain"
        with patch("secondbrain.os.path.relpath", side_effect=ValueError("different drive")):
            create_brain("test", "Test", destination, repository)
        self.assertIn(repository.resolve().as_uri(),
                      (destination / "PROJECT.md").read_text(encoding="utf-8"))

    def test_cli_works_outside_checkout_and_refuses_recreation(self):
        script = Path(__file__).resolve().parents[1] / "secondbrain.py"
        repository = self.root / "Repository ü"
        repository.mkdir()
        original = repository / "original.txt"
        original.write_text("Unverändert", encoding="utf-8")
        destination = self.root / "Neues Gedächtnis"
        command = [sys.executable, str(script), "new", "test", "--name", "Test ü",
                   "--repo", str(repository), "--output", str(destination)]
        first = subprocess.run(command, cwd=self.root, capture_output=True)
        self.assertEqual(first.returncode, 0, first.stderr)
        state = destination / "STATE.md"
        state.write_text("Nutzerwissen", encoding="utf-8")
        second = subprocess.run(command, cwd=self.root, capture_output=True)
        self.assertEqual(second.returncode, 1)
        self.assertEqual(state.read_text(encoding="utf-8"), "Nutzerwissen")
        self.assertEqual(original.read_text(encoding="utf-8"), "Unverändert")


if __name__ == "__main__":
    unittest.main()
