# SecondBrain Aktueller Stand

Stand: 6. Oktober 2026. Diese Übersicht ist eine datierte Zusammenfassung;
aktuelle Nachweise und Grenzen stehen im [Umsetzungsstand](../../docs/STATUS.md).

## Belegter Stand

- Grundlagen und Apple UI-Recherche wurden vor dem UI-Entwurf dokumentiert.
- Der eigene C17-Kern verwaltet Projekte, Markdown, Suche, Archiv und Kontext.
- Die eigene Desktop-App bietet Anlegen, Lesen, Bearbeiten, Speichern, Quellen
  und Projektwechsel mit Schutz ungespeicherter Änderungen und Speicherkonflikte.
- Die [Abnahme zu 287ef48](https://github.com/Lulus792/SecondBrain/actions/runs/37445131476)
  besteht mit 18 Jobs. Desktop Debug/Release und entpackte Pakete bestehen auf
  Windows x64, macOS ARM64 und Linux x64.
- Lokal bestehen alle fünf C-/UI-Prüfungen auch mit AddressSanitizer und
  UndefinedBehaviorSanitizer auf Intel macOS 14.6.1.
- Ein tatsächliches macOS-Archiv besteht nach Entpacken und Verschieben denselben
  Bedienablauf. Das [Paketverfahren](../../docs/DISTRIBUTION.md) ist implementiert.
- Diese Wissensbasis wurde mit dem C-Kern angelegt und mit projektspezifischen
  Inhalten und relativen Originalquellen befüllt.

## Ergebnis der Umsetzung

Version 0.1 ist bereit für den lokalen Arbeitsablauf. Diese eigene Instanz ist in
der App geladen und betrachtet; ihre lokalen Quellen sind erreichbar. Der
Bedienablauf prüft auch Konfliktkopie, Archivierung und Speichern beim Beenden.
Der eigene Projektkontext wird in allen sechs Desktop-Jobs geöffnet und gerendert.
Die Intel-macOS-App liegt lokal unter dist/SecondBrain/secondbrain.app;
weitere Pakete stehen im verlinkten GitHub-Lauf als Actions-Artefakte bereit.

## Neue Umsetzung: Version 0.2

Lumen-Farben, eigene C-Glasdarstellung, reale Sternkarte und Kamerabedienung sind
in die App integriert. Die bisherigen Arbeitsabläufe verwenden weiter denselben
C-Kern. Sichtbarer Fokus, Tab, F6, Pfeile und Bestätigung erlauben reine Tastaturwege.
Der Editor hält seine Undo-Historie getrennt von Suche und Dialogen.
[Bedienvertrag und Prüfumfang](../../docs/UI_TASTATUR.md).

Lokal bestehen sieben Release-Prüfungen und dieselben sieben Prüfungen mit
AddressSanitizer/UndefinedBehaviorSanitizer. Der Tastaturdurchlauf prüft 85 Aussagen,
der bisherige Bedienweg 87. Kleine Fenster mit 150 Prozent Schriftgröße sind darin
enthalten. Die tatsächliche Darstellung wurde in groß und klein betrachtet. Die
entpackte Intel-macOS-App besteht beide Bedienwege aus einem anderen Ordner.
Die [Abnahme zu 5534ff0](https://github.com/Lulus792/SecondBrain/actions/runs/37466622105) besteht mit allen 18 Jobs.
Desktop Debug/Release mit je sieben Prüfungen und entpackte Pakete mit beiden
Bedienwegen bestehen auf Windows x64, macOS ARM64 und Linux x64. Der oben
verlinkte Lauf zu 287ef48 belegt weiterhin die erste Version.

Version 0.2 ist damit für den beauftragten lokalen Arbeitsablauf abgenommen.
Die aktuelle Intel-App liegt unter dist/SecondBrain/secondbrain.app; die weiteren
Pakete stehen im neuen GitHub-Lauf. Neue Erweiterungen werden anhand der
offenen Fragen geplant.

## Grenzen

Keine native Screenreader-Anbindung, automatische Synchronisation oder integrierte
Chat-Anbieter. Grundlegende Markdown-Darstellung; Editor-Undo und Schriftabdeckung
sind begrenzt. Pakete sind nicht durch Apple notarisiert.
Details: [Distribution](../../docs/DISTRIBUTION.md).
