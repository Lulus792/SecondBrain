#!/usr/bin/env python3
"""Create a local project memory from the versioned Markdown template."""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import sys
from datetime import datetime
from urllib.parse import quote


ROOT = Path(__file__).resolve().parent
TEMPLATE = ROOT / "templates" / "brain"
TEMPLATE_VERSION = 1
WINDOWS_RESERVED = {"con", "prn", "aux", "nul"} | {
    f"{prefix}{number}" for prefix in ("com", "lpt") for number in range(1, 10)
}


def create_brain(project_id: str, name: str, destination: Path,
                 repository: Path | None = None) -> Path:
    """Render first, then create an exclusive destination; never replace data."""
    if not re.fullmatch(r"[a-z0-9]+(?:-[a-z0-9]+)*", project_id):
        raise ValueError("Kennung muss Kleinbuchstaben, Zahlen und einzelne Bindestriche enthalten.")
    if project_id in WINDOWS_RESERVED or len(project_id) > 64:
        raise ValueError("Kennung ist unter Windows reserviert oder länger als 64 Zeichen.")
    name = name.strip()
    if not name or any(ord(char) < 32 for char in name):
        raise ValueError("Projektname muss ausgefüllt sein und darf keine Steuerzeichen enthalten.")
    # Do not resolve the final component: a dangling symlink is also existing data.
    destination = Path(os.path.abspath(destination))
    destination = destination.parent.resolve() / destination.name
    if destination.exists() or destination.is_symlink():
        raise FileExistsError(f"Ziel existiert bereits: {destination}")
    if repository is not None:
        repository = repository.resolve()
        if not repository.is_dir():
            raise ValueError(f"Projektordner nicht gefunden: {repository}")

    # Use the user's OS timezone without requiring an IANA database on Windows.
    date = datetime.now().astimezone().date().isoformat()
    relative_repo = None
    repo_text = "Noch kein Projektordner verknüpft."
    source_hint = "Trage hier die Originaldokumente des Projekts ein."
    if repository is not None:
        # Windows repositories on another drive require an absolute reference.
        try:
            relative_repo = os.path.relpath(repository, destination)
            relative_repo = Path(relative_repo).as_posix()
            link = quote(relative_repo, safe="/:")
        except ValueError:
            relative_repo = repository.as_posix()
            link = repository.as_uri()
        repo_text = f"[Verknüpften Projektordner öffnen]({link})."
        source_hint = (f"Projektordner: [Originalquellen]({link}). "
                       "Die Dateien wurden beim Anlegen nicht automatisch eingelesen.")

    replacements = {"@NAME@": name, "@DATE@": date,
                    "@REPO@": repo_text, "@SOURCE_HINT@": source_hint}
    files = sorted(TEMPLATE.rglob("*.md"))
    if not files or not (TEMPLATE / "START.md").is_file():
        raise ValueError(f"Vorlage unvollständig oder nicht gefunden: {TEMPLATE}")
    rendered = {}
    for source in files:
        # One substitution pass preserves any token-like text inside user input.
        content = re.sub(r"@(?:NAME|DATE|REPO|SOURCE_HINT)@",
                         lambda match: replacements[match.group()],
                         source.read_text(encoding="utf-8"))
        rendered[source.relative_to(TEMPLATE)] = content
    metadata = {"schema_version": 1, "template_version": TEMPLATE_VERSION,
                "id": project_id, "name": name, "created": date,
                "project_root": relative_repo}

    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.mkdir()  # Exclusive even if another process creates it meanwhile.
    for relative_path, content in rendered.items():
        target = destination / relative_path
        target.parent.mkdir(parents=True, exist_ok=True)
        with target.open("x", encoding="utf-8", newline="\n") as handle:
            handle.write(content)
    with (destination / "brain.json").open("x", encoding="utf-8", newline="\n") as handle:
        json.dump(metadata, handle, ensure_ascii=False, indent=2)
        handle.write("\n")
    return destination


def main(argv: list[str] | None = None) -> int:
    # Redirected Windows terminals may use a code page that lacks a path's glyphs.
    for stream in (sys.stdout, sys.stderr):
        if hasattr(stream, "reconfigure"):
            stream.reconfigure(errors="backslashreplace")
    parser = argparse.ArgumentParser(description="Ein Second Brain pro Projekt anlegen.")
    commands = parser.add_subparsers(dest="command", required=True)
    new = commands.add_parser("new", help="Neue Wissensbasis aus der Vorlage erstellen.")
    new.add_argument("id", help="Projektkennung, zum Beispiel physim.")
    new.add_argument("--name", help="Anzeigename; standardmäßig die Kennung.")
    new.add_argument("--repo", type=Path, help="Vorhandenen Projektordner verknüpfen.")
    new.add_argument("--output", type=Path, help="Zielordner; standardmäßig brains/KENNUNG.")
    args = parser.parse_args(argv)
    try:
        destination = create_brain(args.id, args.name or args.id,
                                   args.output or ROOT / "brains" / args.id, args.repo)
    except (OSError, ValueError) as error:
        print(f"Nicht erstellt: {error}", file=sys.stderr)
        print("Bei einem Schreibfehler kann ein unvollständiger neuer Zielordner bestehen bleiben.",
              file=sys.stderr)
        return 1
    print(f"Second Brain erstellt: {destination}")
    print(f"Einstieg: {destination / 'START.md'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
