# Architektur der C Anwendung

SecondBrain wird in C17 aufgebaut. Der fachliche Kern enthält keine externen
Bibliotheken und keine Abhängigkeit von der Oberfläche.

## Kern und Plattformanbindung

Der Kern verwaltet Arbeitsordner, Projekte und Markdown-Dokumente. Er erzeugt
die vorhandene Grundstruktur, liest und schreibt Dateien, durchsucht Inhalte
und stellt KI-Kontext zusammen. Dateiverweise und Metadaten bleiben lesbar.
Die bisherigen Vorlagen und das Physim-Beispiel können weiterverwendet werden.

Eine eigene kleine Plattformschicht verbindet Dateizugriff und Verzeichnislisten
mit Windows beziehungsweise POSIX auf macOS und Linux. Pfade werden in der
Anwendung als UTF-8 geführt; Windows-Systemaufrufe erhalten UTF-16.
Speichern erfolgt über eine temporäre Datei und atomaren Austausch. Ein
Inhaltsabgleich schützt vor dem Überschreiben inzwischen veränderter Originale.

## UI Abhängigkeiten

SDL3 übernimmt ausschließlich Fenster, Rendering, Eingabe, Zwischenablage und
UI-Dialoge. Nuklear stellt die in C geschriebenen Oberflächenkomponenten und
Textbearbeitung bereit. Schriftdateien sind UI-Assets. Diese Abhängigkeiten
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
