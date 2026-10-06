# SecondBrain Projektauftrag

Stand: 6. Oktober 2026. Grundlage: [verbindlicher Projektplan](../../docs/PROJEKTPLAN.md).

## Ziel

Eine eigene Desktop-Anwendung in C, die für jedes Projekt ein wiederverwendbares
Second Brain anlegt und verwaltet. Mensch und KI verwenden dieselben lesbaren
Projektziele, Entscheidungen, Quellen, Wissensnotizen und Arbeitsübergaben.
Diese Instanz dokumentiert SecondBrain selbst.

## Verbindliche Anforderungen

- Windows, macOS und Linux sind gleichwertige Zielplattformen.
- C implementiert den fachlichen Kern. Externe Bibliotheken sind nur für UI zulässig.
- Anlegen, Anzeigen, Bearbeiten, Suchen und Verwalten erfolgen in der eigenen App.
- Vor dem UI-Entwurf wird UI-Design studiert, mit Orientierung an Apples Gestaltung.
- Jeder abgeschlossene und geprüfte Schritt wird committet und zu GitHub gepusht.
- Bestehendes Projektwissen bleibt erhalten; neue Instanzen überschreiben keine Ordner.
- Für SecondBrain selbst wird ein eigenes Projektgedächtnis erstellt und verwendet.

## Erfolgskriterien der ersten Version

- Projekt und Notiz lassen sich in der App anlegen, bearbeiten, speichern und wieder öffnen.
- Suche, Projektwechsel, lokale Quellen, Archiv und kopierbarer KI-Kontext funktionieren.
- Ungespeicherte Änderungen und externe Speicherkonflikte gehen nicht still verloren.
- Die tatsächlichen Abläufe bestehen auf allen drei Systemen.
- Entpackte Pakete starten ohne Python, Compiler oder externe Wissens-App.
- Das eigene Projektgedächtnis nennt belegten Stand, nächste Schritte und Originalquellen.

## Umfang

Version 0.1 verwendet lokale UTF-8-Markdown-Dateien und JSON-Metadaten.
KI-Kontext und Dateizugriff sind der erste gemeinsame Zugang. Automatische
KI-Pflege, Synchronisation, Chat-Anbieter und Vorlagenmigration sind mögliche
spätere Erweiterungen, keine bereits implementierten Funktionen.

## Arbeitsumgebung

[Repository-Ordner](../..), [GitHub](https://github.com/Lulus792/SecondBrain).
Originale: [Quellenindex](SOURCES.md). Projektanweisungen:
[AGENTS.md des Repositories](../../AGENTS.md).
