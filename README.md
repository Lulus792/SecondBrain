# SecondBrain für deine Projekte

SecondBrain ist eine eigene Desktop-Anwendung in C für eine gemeinsame Wissensbasis
von dir und der KI. Sie legt Projektgedächtnisse an, zeigt sie an und unterstützt
Bearbeitung, Suche und Verwaltung. Jedes Projekt bekommt eigene Ziele, Entscheidungen,
Quellen und Notizen auf Grundlage einer wiederverwendbaren Struktur.

Die verbindlichen Anforderungen stehen im [Projektplan](docs/PROJEKTPLAN.md):
Windows, macOS und Linux; externe Bibliotheken ausschließlich für die UI;
UI-Recherche vor dem Entwurf und Gestaltung nach Apples Human Interface Guidelines.
Die normale Nutzung erfolgt in der eigenen Anwendung.

Version 0.2 integriert die ausgewählte Lumen-Sternkarte und schwebende Glaskarten
in die C-App. Dokumente, echte Markdown-Verweise, Kamera und sämtliche bisherigen
Arbeitsabläufe sind verbunden. Die Oberfläche bietet durchgängige Tastaturwege
mit sichtbarem Fokus. Aktuelle Prüfungen und ihre Plattformgrenzen stehen im
[Umsetzungsstand](docs/STATUS.md); die vorherige Version ist bereits auf allen
drei Systemen einschließlich entpackter Pakete geprüft.

[GitHub Repository](https://github.com/Lulus792/SecondBrain) ·
[Automatisierte Plattformprüfungen](https://github.com/Lulus792/SecondBrain/actions)

## Eigene Desktop-Anwendung

Fertige Pakete werden nach erfolgreichen Release-Prüfungen als
`SecondBrain-Windows-X64`, `SecondBrain-macOS-ARM64` und `SecondBrain-Linux-X64`
für Version 0.1 im [geprüften GitHub-Actions-Lauf](https://github.com/Lulus792/SecondBrain/actions/runs/37445131476)
bereitgestellt. Den Download entpacken, dann das darin enthaltene
Anwendungsarchiv entpacken. Die Anleitung `QUICKSTART.txt` gehört zum Paket.
Ein GitHub-Login kann für den Download der Actions-Artefakte erforderlich sein.
Die Pakete bleiben dort 30 Tage erhalten.

Voraussetzungen zum Bauen: C17-Compiler, CMake ab 3.20 und auf Linux die
Entwicklungsdateien des Fenstersystems. SDL3 wird bei Bedarf beim Build aus der
festgelegten Originalversion geladen. Die App läuft ohne Python.

```sh
cmake -S . -B build/app -DCMAKE_BUILD_TYPE=Release
cmake --build build/app --config Release --parallel 4
ctest --test-dir build/app -C Release --output-on-failure
```

Unter macOS entsteht `build/app/secondbrain.app`; unter Linux
`build/app/secondbrain`, unter Windows bei Visual Studio
`build/app/Release/secondbrain.exe`. Start per Doppelklick oder Terminal.
Ohne Argumente verwendet die App den Ordner `SecondBrain` in deinem
Benutzerverzeichnis. Über die Projektwahl oder Command/Control+O kannst du einen anderen Ordner
mit Projektgedächtnissen auswählen. Das vorhandene Beispiel liegt in `brains`:

```sh
# macOS
build/app/secondbrain.app/Contents/MacOS/secondbrain --workspace brains
# Linux
build/app/secondbrain --workspace brains
```

```powershell
# Windows
.\build\app\Release\secondbrain.exe --workspace brains
```

Die Oberfläche bietet Projekte und Wissensbereiche, Dokumentliste, Suche,
Lesemodus, Markdown-Bearbeitung, neue Notizen, Archiv und KI-Kontext zum Kopieren.
Lokale Textquellen und verlinkte Ordner öffnen sich schreibgeschützt in der App.
Vor einem Wechsel oder dem Beenden schützt ein Dialog ungespeicherte Änderungen.
Bei externen Dateiänderungen bleibt deine eigene Fassung erhalten; sie lässt
sich als neue Notiz sichern. Darstellung und Schriftgröße sind anpassbar.
Die Sternkarte zeigt Dokumente des aktuellen Projekts. Ihre Linien stehen für
vorhandene interne Markdown-Verweise; die Farben markieren Wissensbereiche.
„Liste“ und Suche öffnen die Dokumentauswahl. Ziehen dreht die Kamera,
Umschalt+Ziehen verschiebt sie; das Mausrad zoomt. Die Glaseffekte sind eine eigene
plattformübergreifende Nachbildung, keine native Apple-Materialkomponente.

Tab / Umschalt+Tab erreicht die verfügbaren Elemente; Enter / Leertaste aktiviert.
F6 wechselt Werkzeuge, Sternkarte und Dokument. In der Karte wählen Pfeile Sterne,
Enter öffnet eine Notiz. Umschalt+Pfeile dreht die Kamera, +/- zoomt, Pos1 setzt sie
zurück. Bild auf/ab scrollt Lesebereiche. Tab verlässt den Editor; Ctrl+I fügt einen
Tabulator ein. F1 zeigt die vollständige Hilfe. Command gilt für die übrigen
Kürzel auf macOS, Control auf Windows und Linux.

## Separater C-Kern und Strukturprototyp

Paketerstellung, Voraussetzungen und Grenzen stehen in
[Distribution](docs/DISTRIBUTION.md).

Der C-Kern lässt sich bereits unabhängig von UI-Bibliotheken bauen:

```sh
cmake -S . -B build/core -DSB_BUILD_UI=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build/core --config Debug
ctest --test-dir build/core -C Debug --output-on-failure
```

Das native Entwicklungswerkzeug `secondbrain-cli` liegt im Buildordner, unter
Windows bei Visual-Studio-Builds im Unterordner `Debug`. Es unterstützt `new`,
`list`, `search` und `context`; Aufruf ohne Argumente zeigt die Syntax.
Zur Laufzeit benötigt es kein Python.

Die folgenden Befehle betreffen den vorhandenen Python-Prototyp. Für ihn genügt
Python 3.10 oder neuer; er hat keine zusätzlichen Paketabhängigkeiten. Das Datum
stammt aus der lokal eingestellten Betriebssystem-Zeitzone. Die C-Anwendung
ermöglicht diese Arbeit durch ihre eigene Oberfläche.

Im Ordner dieses Repositories unter macOS oder Linux:

```sh
python3 secondbrain.py new mein-projekt --name "Mein Projekt"
```

Unter Windows in PowerShell:

```powershell
py -3 secondbrain.py new mein-projekt --name "Mein Projekt"
```

Falls Python dort über `python` verfügbar ist, kann dieser Befehl entsprechend
verwendet werden. Alle Optionen sind auf den drei Plattformen gleich.

Das erstellt `brains/mein-projekt/`. Der Bezeichner darf Kleinbuchstaben, Zahlen
und einzelne Bindestriche enthalten, höchstens 64 Zeichen. Unter Windows
reservierte Gerätenamen werden auf allen Plattformen abgelehnt; der Anzeigename
ist frei wählbar.

Ein bestehendes Projekt verknüpfen:

```sh
python3 secondbrain.py new anderes-projekt --name "Anderes Projekt" --repo ../anderes-projekt
```

Ein eigenes Zielverzeichnis wählen:

```sh
python3 secondbrain.py new mein-projekt --name "Mein Projekt" --output /pfad/zum/projekt/second-brain
```

Der Generator verweigert jedes bereits vorhandene Zielverzeichnis. `--repo`
speichert einen Verweis auf das Projekt; eine automatische Übernahme oder
Auswertung von dessen Dateien erfolgt nicht. Relative Verweise bleiben bei
gemeinsamem Verschieben erhalten; bei getrenntem Verschieben musst du sie anpassen.

## Mit einem Projekt anfangen

1. `START.md` öffnen und in `PROJECT.md` Ziel und Erfolgskriterien ergänzen.
2. In `SOURCES.md` die verbindlichen Originaldokumente eintragen.
3. In `STATE.md` den aktuellen Stand und einen konkreten nächsten Schritt festhalten.
4. Ideen zunächst in `inbox/INBOX.md` sammeln.
5. Nach einem Arbeitsabschnitt relevante Ergebnisse und Entscheidungen einpflegen.

Für einen neuen KI-Chat kannst du diesen Auftrag verwenden und den Pfad ersetzen:

> Lies /absoluter/pfad/zum/second-brain/START.md und die darin genannten
> Kerndateien. Öffne anschließend die Quellen, die für meine Aufgabe relevant
> sind. Sage, was belegt, offen oder veraltet ist, und arbeite dann an meinem Auftrag.

Die Dateien werden dadurch zum externen Projektgedächtnis. Ihre Existenz allein
stellt weder ein automatisches Einlesen noch eine laufende Aktualisierung sicher.
Die Pflege erfolgt in dieser Version durch dich oder durch einen damit beauftragten
KI-Chat. Die jeweiligen Projektanweisungen bleiben maßgeblich.

## Das Physim Beispiel

SecondBrain besitzt auch ein [eigenes Projektgedächtnis](brains/secondbrain/START.md)
mit Anforderungen, Entscheidungen, Stand, Originalquellen und Wissensnotizen.
Es erscheint als Projekt „SecondBrain“, wenn du die App mit `--workspace brains`
startest. `--project secondbrain` wählt es direkt aus.

[Physim öffnen](brains/physim/START.md). Das Beispiel verknüpft dein vorhandenes
Repository und enthält einen ersten Projektauftrag, einen Quellenindex sowie
einen vorsichtigen Überblick. Stand: 6. Oktober 2026. Aussagen zum Funktionsumfang
sind aus der vorhandenen Projektdokumentation übernommen, keine neuen Testergebnisse.
Die lokalen Quellenverweise erwarten `SecondBrain/` und `physim/` nebeneinander
im selben übergeordneten Ordner. Auf einem anderen Rechner beide Repositories
entsprechend ablegen oder die Verweise anpassen.

## Konzept und Grundlagen

- [Grundlagen und recherchierte Quellen](docs/GRUNDLAGEN.md)
- [Verbindlicher Plan der C-Anwendung und UI-Recherche](docs/PROJEKTPLAN.md)
- [Aufbau und Arbeitsweise unseres Systems](docs/KONZEPT.md)
- [Wiederverwendbare Vorlage](templates/brain/START.md)

## Prüfen

```sh
python3 -m unittest discover -s tests -v
```

Unter Windows: `py -3 -m unittest discover -s tests -v`.
GitHub Actions führt diese Prüfungen nach jedem Push für Windows, macOS und Linux
mit Python 3.10 und 3.14 aus. Die Tests prüfen den Generator und die Kommandozeile,
Unicode-Pfade, Verweise und den Schutz vorhandener Inhalte. Ein Symlink-Test
wird nur ausgelassen, wenn das Betriebssystem keine Symlinks erlaubt.

Die dokumentierten Ergebnisse stehen in [Plattformprüfung](docs/PLATTFORMEN.md).
Die Nachweise unterscheiden Python-Prototyp, C-Kern, UI-Bausteine,
Desktop-Bedienablauf und entpackte Pakete anhand tatsächlich ausgeführter Prüfungen.

## Änderungen veröffentlichen

Jeder abgeschlossene und geprüfte Arbeitsschritt wird mit einer verständlichen
Beschreibung committet und zu `origin` auf GitHub gepusht. Diese Vorgabe gilt
auch für weitere Arbeit in diesem Repository und ist in AGENTS.md festgehalten.
Prüfergebnisse und Einschränkungen werden mit dem jeweiligen Stand dokumentiert.

Bestehende Instanzen behalten ihre ursprüngliche Vorlagenversion. Gemeinsames
Wissen kannst du später projektübergreifend verlinken; projektübergreifende Suche,
Synchronisation, Chat-Anbindung und Vorlagenmigration sind mögliche spätere
Erweiterungen.
