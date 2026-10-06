# SecondBrain

[![Plattformprüfungen](https://github.com/Lulus792/SecondBrain/actions/workflows/tests.yml/badge.svg)](https://github.com/Lulus792/SecondBrain/actions/workflows/tests.yml)

Dein Projektwissen an einem Ort: Ziele, Notizen, Entscheidungen und Quellen.
SecondBrain ist eine Desktop-App für Windows, macOS und Linux, geschrieben in C.
Jedes Projekt bekommt ein eigenes Gedächtnis, das du und eine KI gemeinsam nutzen
könnt. Die Dateien bleiben als Markdown und kleine JSON-Metadaten lesbar.

![SecondBrain: Sternkarte und schwebende Notizkarte](docs/images/secondbrain.png)

**In Entwicklung.** Die lokale Arbeit mit Projekten ist umgesetzt. Die App hat
noch keinen stabilen 1.0-Release. Aktuelle Nachweise stehen im
[Umsetzungsstand](docs/STATUS.md), offene Aufgaben in der [Release-Liste](docs/RELEASE.md).
Die Vorschau oben stammt aus der echten C-App.

## Was du damit machen kannst

- Projekte anlegen, zwischen ihnen wechseln und Wissen nach Bereichen ordnen.
- Notizen schreiben, bearbeiten, durchsuchen und archivieren.
- Dokumente als Sterne erkunden; Linien zeigen vorhandene interne Markdown-Links.
- Lokale Quellen und verlinkte Ordner direkt in der App lesen.
- Gespeicherten Projektkontext für einen KI-Chat kopieren.
- Die App vollständig mit der Tastatur bedienen.

Die dunkle Lumen-Oberfläche verbindet leuchtende Sterne mit schwebenden
Glaskarten. Schriftgröße, helle Darstellung, reduzierte Transparenz und reduzierte
Bewegung lassen sich in den Einstellungen wählen. Die Glasdarstellung ist eine
eigene Umsetzung nach Apple-Vorbild.

## Starten

Die [GitHub-Actions-Läufe](https://github.com/Lulus792/SecondBrain/actions/workflows/tests.yml)
stellen nach bestandenen Release-Prüfungen Pakete bereit:
`SecondBrain-Windows-X64`, `SecondBrain-macOS-ARM64` und `SecondBrain-Linux-X64`.
Entpacke den Download und das darin enthaltene Anwendungsarchiv. Die Datei
`QUICKSTART.txt` erklärt den Start auf deinem System. Downloads erfordern unter
Umständen einen GitHub-Login und bleiben 30 Tage verfügbar.

Ohne Argumente verwendet die App `SecondBrain` in deinem Benutzerordner.
Lege über **Projekte → Neues Projekt** ein Gedächtnis an. Über die Projektwahl
oder Command/Control+O kannst du einen bestehenden Arbeitsordner öffnen.
Ein externer Wissenseditor und Python sind für die App nicht erforderlich.

Die Pakete sind noch nicht mit Herausgeberzertifikaten signiert oder durch Apple
notarisiert. Voraussetzungen und Paketaufbau: [Distribution](docs/DISTRIBUTION.md).

## Selbst bauen

Du brauchst einen C17-Compiler und CMake ab 3.20. Auf macOS eignet sich der
Compiler der Xcode Command Line Tools, auf Windows Visual Studio mit C-Werkzeugen,
auf Linux GCC oder Clang sowie die Entwicklungsdateien des Fenstersystems.
SDL3 wird bei Bedarf in einer festgelegten Version beim Build geladen.
[Plattformdetails](docs/DISTRIBUTION.md).

```sh
cmake -S . -B build/app -DCMAKE_BUILD_TYPE=Release
cmake --build build/app --config Release --parallel 4
ctest --test-dir build/app -C Release --output-on-failure
```

| System | Anwendung |
| --- | --- |
| macOS | `build/app/secondbrain.app` |
| Windows, Visual Studio | `build/app/Release/secondbrain.exe` |
| Linux | `build/app/secondbrain` |

Starte die Anwendung per Doppelklick. Um das Projektgedächtnis dieses Repositories
zu öffnen, verwende im Terminal aus dem Repository-Ordner:

```sh
# macOS
build/app/secondbrain.app/Contents/MacOS/secondbrain --workspace brains --project secondbrain
# Linux
build/app/secondbrain --workspace brains --project secondbrain
```

```powershell
# Windows
.\build\app\Release\secondbrain.exe --workspace brains --project secondbrain
```

## Bedienung

| Aktion | Taste |
| --- | --- |
| Bedienelement wechseln | Tab / Umschalt+Tab |
| Werkzeuge, Sternkarte, Dokument wechseln | F6 / Umschalt+F6 |
| Nächste Notiz in der Sternkarte sofort öffnen | Pfeiltasten |
| Kamera drehen | Umschalt+Pfeile |
| Zoomen / Kamera zurücksetzen | +, − / Pos1 |
| Lesebereich scrollen | Bild auf / Bild ab |
| Speichern / Suche / Bearbeiten | Command/Control+S / F / E |
| Neue Notiz / neues Projekt | Command/Control+N / Umschalt+N |
| Zurück oder abbrechen / Hilfe | Escape / F1 |

Command gilt auf macOS, Control auf Windows und Linux. Tab verlässt auch den
Editor; Ctrl+I fügt dort einen Tabulator ein. Mit der Maus: Ziehen dreht die
Sternkarte, Umschalt+Ziehen verschiebt sie, das Mausrad zoomt. Über einer Karte
scrollt es den Inhalt. Weitere Kürzel stehen in der eingebauten Hilfe und im
[Tastaturvertrag](docs/UI_TASTATUR.md).

Vor einem Dokumentwechsel oder dem Beenden fragt die App nach ungespeicherten
Änderungen. Bei einer extern geänderten Datei kannst du deine Fassung als neue
Notiz sichern, ohne die fremde Fassung zu überschreiben.

## Dein Projektgedächtnis

`START.md` führt durch Auftrag, Stand, Entscheidungen, offene Fragen und Quellen.
`knowledge/`, `inbox/`, `journal/` und `archive/` enthalten die weiteren Notizen.
Trage zuerst Ziel und Erfolgskriterien in `PROJECT.md` ein und ergänze die
Originalquellen in `SOURCES.md`.

Ein KI-Chat kann die Dateien direkt lesen oder den kopierten Kontext verwenden.
Die Pflege beauftragst du ausdrücklich; sie erfolgt noch nicht automatisch.
Das [eigene Projektgedächtnis](brains/secondbrain/START.md) zeigt die Struktur
im Einsatz. [Konzept und Arbeitsweise](docs/KONZEPT.md).

## Entwicklung

Der fachliche Kern verwendet C und Betriebssystem-APIs. SDL3 und Nuklear werden
für die UI eingesetzt. Die Herkunft und Lizenzen der Abhängigkeiten stehen unter
[third_party](third_party/README.md); die Architektur in [docs/ARCHITEKTUR.md](docs/ARCHITEKTUR.md).
Eine Projektlizenz für den eigenen Code muss vor dem öffentlichen 1.0-Release
noch festgelegt werden.

Den Kern kannst du separat bauen:

```sh
cmake -S . -B build/core -DSB_BUILD_UI=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build/core --config Debug
ctest --test-dir build/core -C Debug --output-on-failure
```

`secondbrain-cli` unterstützt `new`, `list`, `search` und `context`.
Der ältere Python-Generator ist als [Strukturprototyp](docs/STRUKTURPROTOTYP.md)
dokumentiert. Neue Vorlagen überschreiben keine bestehenden Instanzen.
Abgeschlossene, geprüfte Arbeitsschritte werden committet und nach GitHub gepusht.

- [Projektplan und Todos](docs/PROJEKTPLAN.md)
- [Aktueller Stand und Plattformnachweise](docs/STATUS.md)
- [Aufgaben bis zum vollständigen Release](docs/RELEASE.md)
- [UI-Recherche](docs/UI_RECHERCHE.md) und [Gestaltung](docs/UI_GALAXIE.md)
- [Grundlagen des Second-Brain-Konzepts](docs/GRUNDLAGEN.md)

Fehlerberichte sollten Betriebssystem, Version, Schritte zum Wiederholen und das
beobachtete Verhalten enthalten. [GitHub Issues](https://github.com/Lulus792/SecondBrain/issues).
