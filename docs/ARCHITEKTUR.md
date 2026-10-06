# Architektur der C Anwendung

SecondBrain wird in C17 aufgebaut. Der fachliche Kern enthält keine externen
Bibliotheken und keine Abhängigkeit von der Oberfläche.

## Kern und Plattformanbindung

Der Kern verwaltet Arbeitsordner, Projekte und Markdown-Dokumente. Er erzeugt
die vorhandene Grundstruktur, liest und schreibt Dateien, durchsucht Inhalte
und stellt KI-Kontext zusammen. Dateiverweise und Metadaten bleiben lesbar.
Die vorhandenen Vorlagen und Projektgedächtnisse können weiterverwendet werden.

Eine eigene kleine Plattformschicht verbindet Dateizugriff und Verzeichnislisten
mit Windows beziehungsweise POSIX auf macOS und Linux. Pfade werden in der
Anwendung als UTF-8 geführt; Windows-Systemaufrufe erhalten UTF-16.
Speichern erfolgt über eine temporäre Datei und atomaren Austausch. Ein
Inhaltsabgleich schützt vor dem Überschreiben inzwischen veränderter Originale.

## UI Abhängigkeiten

SDL3 übernimmt ausschließlich Fenster, Rendering, Eingabe, Zwischenablage und
UI-Dialoge. Nuklear stellt die in C geschriebenen Oberflächenkomponenten und
Textbearbeitung bereit. SDL_ttf formt und rastert Text mit HarfBuzz und FreeType;
ein eigener C-Hook fügt Texttexturen in die vorhandene Zeichenreihenfolge ein.
[Schriftrollen und Grenzen](TEXTDARSTELLUNG.md). AccessKit stellt die native Zugänglichkeitsschicht bereit:
macOS Accessibility, Windows UI Automation und Linux AT-SPI. Ein eigener C-Adapter
veröffentlicht UI-Snapshots und verarbeitet native Aktionen auf dem UI-Thread.
Seine Kontextkennungen verwerfen Aktionen aus inzwischen gewechselten Dokumenten.
Schriftdateien sind UI-Assets. Diese Abhängigkeiten
werden ausschließlich in das App-Ziel eingebunden.

Quellen: [SDL3](https://wiki.libsdl.org/SDL3/FrontPage),
[Nuklear](https://github.com/Immediate-Mode-UI/Nuklear).
Versionen, Lizenzen und lokale Anpassungen werden unter third_party dokumentiert.

## Build und Distribution

CMake erzeugt native Builds für Windows, macOS und Linux. CMake ist ein
Entwicklungswerkzeug, keine Bibliotheks- oder Laufzeitabhängigkeit des Kerns.
Der Kern lässt sich separat ohne UI bauen und testen. Für die Anwendung wird
SDL statisch gebaut; Vorlagen und Schriftassets gehören zum Paket.

Die App benötigt zur Laufzeit weder Python noch Obsidian. Die vorhandenen
Python-Prüfungen bleiben als Nachweis des älteren Strukturprototyps erhalten.
Neue C- und App-Prüfungen belegen die tatsächliche Anwendung separat.

## Räumliche Ansicht und Tastatur

app/graph.c gehört zur UI-unabhängigen C-Anwendungsschicht. Es liest gespeicherte
Markdown-Dokumente, normalisiert relative interne Verweise und erzeugt einen
stabilen räumlichen Grundaufbau. Das Limit der Sternkarte liegt bei 4096 Dokumenten
und 65536 Verweisen; die paginierte Dokumentliste bleibt unabhängig verfügbar.
Unterstützt werden einfache Inline-Links, einschließlich Fragmenten und
Prozentkodierung; Bilder, Code und externe Ziele bilden keine Sternkanten.

app/space.c ist der eigene C-UI-Renderer. Er rastert Sterne und Verbindungen und
berechnet Glasflächen mit Hintergrundbrechung und Lichtkanten. Der Materialraster
ist auf 1600 × 1100 begrenzt; Nuklear stellt Schrift und Bedienflächen separat in
der Fensterauflösung dar. Ein Szenenfingerabdruck vermeidet unveränderte Berechnungen.
SDL bleibt die einzige Rendering-/Fensterbibliothek. Native Apple-Liquid-Glass-APIs
werden nicht verwendet.

app/desktop.c verwaltet stabile Fokuskennungen, die Reihenfolge der Gruppen,
Dialogfokus und dessen Rückkehr. F6 und Tab bieten getrennte Wege für Gruppen
und einzelne Elemente. Der Dokumenteditor verwendet einen dauerhaften
nk_text_edit-Zustand über dem Modellpuffer; Such- und Dialogfelder teilen seine
Undo-Historie nicht. Dokumentwechsel setzen diese Historie zurück.
app/keyboard_test.c prüft die tatsächlichen Wege ausschließlich mit SDL-Tastatur-
und Zwischenablageereignissen. Die Paketprüfung führt beide Bedienprüfungen aus.

Der [Datenvertrag](DATENVERTRAG.md) legt Metadaten, Textgrenzen und den
Bestandsschutz beim Update fest. Metadatenliste und Sicherungsprüfung verwenden
dieselbe eigene C-Prüfung; Textimporte lehnen NUL vor dem Editor ab.
