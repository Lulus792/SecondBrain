# SecondBrain für deine Projekte

Hier entsteht eine eigene Anwendung in C für eine gemeinsame Wissensbasis von
dir und der KI. Sie soll Projektgedächtnisse selbst anlegen, anzeigen, bearbeiten,
durchsuchen und verwalten. Jedes Projekt bekommt eigene Ziele, Entscheidungen,
Quellen und Notizen auf Grundlage einer wiederverwendbaren Struktur.

Die verbindlichen Anforderungen stehen im [Projektplan](docs/PROJEKTPLAN.md):
Windows, macOS und Linux; externe Bibliotheken ausschließlich für die UI;
UI-Recherche vor dem Entwurf und Gestaltung nach Apples Human Interface Guidelines.
Die normale Nutzung soll vollständig in der eigenen Anwendung möglich sein.

Die Umsetzung läuft. UI-Recherche und Oberflächenentwurf sind dokumentiert;
ein unabhängiger C17-Kern und ein natives Entwicklungswerkzeug sind implementiert.
Die UI-Grundlage mit Schriften, Darstellung und Texteditor ist ebenfalls
implementiert und auf allen drei Plattformen geprüft.
Die eigene Oberfläche und die vollständige App-Abnahme stehen noch aus.
Der aktuelle Nachweis steht im [Umsetzungsstand](docs/STATUS.md).

[GitHub Repository](https://github.com/Lulus792/SecondBrain) ·
[Automatisierte Plattformprüfungen](https://github.com/Lulus792/SecondBrain/actions)

## Vorhandener Strukturprototyp

Der C-Kern lässt sich bereits unabhängig von UI-Bibliotheken bauen:

```sh
cmake -S . -B build/core -DCMAKE_BUILD_TYPE=Debug
cmake --build build/core --config Debug
ctest --test-dir build/core -C Debug --output-on-failure
```

Das native Entwicklungswerkzeug `secondbrain-cli` liegt im Buildordner, unter
Windows bei Visual-Studio-Builds im Unterordner `Debug`. Es unterstützt `new`,
`list`, `search` und `context`; Aufruf ohne Argumente zeigt die Syntax.
Zur Laufzeit benötigt es kein Python.

Die folgenden Befehle betreffen den vorhandenen Python-Prototyp. Für ihn genügt
Python 3.10 oder neuer; er hat keine zusätzlichen Paketabhängigkeiten. Das Datum
stammt aus der lokal eingestellten Betriebssystem-Zeitzone. Die geplante
C-Anwendung soll diese Arbeit durch ihre eigene Oberfläche ermöglichen.

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
Sie gelten für den Python-Prototyp. Die zukünftige C-Anwendung und ihre Oberfläche
benötigen eigene ausgeführte Prüfungen auf allen drei Plattformen.

## Änderungen veröffentlichen

Jeder abgeschlossene und geprüfte Arbeitsschritt wird mit einer verständlichen
Beschreibung committet und zu `origin` auf GitHub gepusht. Diese Vorgabe gilt
auch für weitere Arbeit in diesem Repository und ist in AGENTS.md festgehalten.
Prüfergebnisse und Einschränkungen werden mit dem jeweiligen Stand dokumentiert.

Bestehende Instanzen behalten ihre ursprüngliche Vorlagenversion. Gemeinsames
Wissen kannst du später projektübergreifend verlinken; automatische Suche,
Synchronisation, Chat-Anbindung und Vorlagenmigration sind mögliche spätere
Erweiterungen.
