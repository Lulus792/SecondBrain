# SecondBrain: aktueller Stand

Stand: 7. Oktober 2026. Originale: [Umsetzungsstand](../../docs/STATUS.md),
[Plattformnachweise](../../docs/PLATTFORMEN.md) und [Release-Liste](../../docs/RELEASE.md).
Historische Arbeitsabschnitte bleiben im [Journal](journal/2026-10-06.md).

## Implementiert und belegt

- Eigene C17-App: Projekte und Notizen anlegen, lesen, bearbeiten, suchen und
  archivieren; Quellen lesen und gespeicherten KI-Kontext kopieren. Entwürfe und
  erkannte Speicherkonflikte bleiben geschützt.
- Lumen-Sternkarte und eigene Glasdarstellung/Icons; direkte Pfeilnavigation,
  Raumfahrt, begrenztes weiches Scrollen, Menüpfeile und große Leseansicht.
- Eigene Sicherungsarchive mit SHA-256, Vorschau, Hintergrundarbeit und Abbruch;
  Wiederherstellung veröffentlicht ausschließlich einen freien neuen Projektordner.
  [Vertrag](../../docs/SICHERUNG.md).
- Einstellungen, letzter Arbeitsordner und Projektposition bleiben gespeichert.
  Native Dialoge sind angebunden. Eigene Auswahl bleibt getrennt von bekannten
  OS-Vorgaben für Darstellung, Bewegung, Transparenz und Kontrast.
- AccessKit ausschließlich in der UI; tatsächliche native Provider-/UIA-/AT-SPI-
  Abfragen und Aktionen bestehen. Die Linux-Cache-Signalstruktur ist korrigiert.
  [Umfang und Grenzen](../../docs/BARRIEREFREIHEIT_PLAN.md).
- [0.6.1-Abnahme zu b064bd5](https://github.com/Lulus792/SecondBrain/actions/runs/37529776082):
  18 erfolgreiche Jobs, 16 Desktoptests je Debug/Release auf Windows x64,
  macOS ARM64 und Linux x64 sowie drei entpackte Pakete. Raster- und Fokuskontrast
  sind gezielt nachgeprüft. dist/SecondBrain ist lokal auf 0.8.0 (geprüfter Entwicklungsbuild).

## Abgenommener erster Start

0.7.0 bündelt im leeren Zustand Anlegen, Öffnen, Wiederherstellung, Hilfe und
Darstellung. Große Schrift scrollt die Aktionen unter einer festen Überschrift.
Dialogabbruch erhält den Ursprung. Alle 17 lokalen Release-Tests bestehen;
die abschließende Mausradprüfung und gezielte ASan/UBSan-Prüfungen bestehen ebenfalls.
Die [Abnahme zu beba1e5](https://github.com/Lulus792/SecondBrain/actions/runs/37531214816)
besteht mit 18 Jobs, 17 Desktoptests je Debug/Release und drei entpackten Paketen.
[Recherche und Vertrag](../../docs/ERSTER_START.md).

## Laufende Arbeit: Datenvertrag

0.7.1 prüft Metadatenfelder und unbekannte Schemas konsistent und verhindert
NUL-bedingte Textverkürzung. Alle acht UI-unabhängigen Kerntests und gezielte
ASan/UBSan-Prüfungen sowie abschließende Desktop-Nachprüfungen bestehen.
Die Regression schlägt beim bisherigen Kern fehl. Die [Abnahme zu b3c0af5](https://github.com/Lulus792/SecondBrain/actions/runs/37532681504)
besteht mit 18 Jobs, 18 Desktoptests je Debug/Release und drei entpackten Paketen.
[Datenvertrag](../../docs/DATENVERTRAG.md).

0.7.2 erhält einzelne Projektfehler als sichtbare Einträge und lässt andere
Projekte nutzbar. Alle 20 Release-Tests und der zusätzliche CLI-Prozesstest
bestehen, ebenso zehn reine Kerntests und vier ASan/UBSan-Wege. Erneutes Prüfen
und Fokus-Reveal nach Größenänderung sind betrachtet. Die [Abnahme zu f2e2725](https://github.com/Lulus792/SecondBrain/actions/runs/37534667024)
besteht mit 18 Jobs, 21 Desktoptests je Debug/Release und drei entpackten Paketen.

0.7.3 prüft aktuelle Metadaten zusätzlich vor Schreibaktionen, auch bei bereits
geöffnetem Projekt. Elf reine Kerntests bestehen; Save-Guard, Entwurf und
Originaldatei bleiben bei erkannten Metadatenfehlern erhalten. 22 Release-Tests,
elf Kerntests und gezielte ASan/UBSan-/UI-Nachprüfungen bestehen. Die [Abnahme
zu 7331eca](https://github.com/Lulus792/SecondBrain/actions/runs/37536230101) besteht
mit 18 Jobs, 22 Desktoptests je Debug/Release und drei entpackten Paketen.

## Laufende Arbeit: dauerhafte Distribution

Die vorbereitete Vorabversions-Pipeline verwendet dieselben Plattformtests,
plant zusätzlich Intel-macOS-Pakete und verifiziert Uploads anhand der Prüfsummen
vor Veröffentlichung. Alle zwölf Kerntests einschließlich Tag-/Manifestprüfung und Workflow-Lint
bestehen lokal;
tatsächliche Erstveröffentlichung und Intel-CI-Abnahme stehen noch aus.

## Weiterarbeiten und Grenzen

Der vollständige Auftrag vor 1.0 bleibt aktiv. Nächste Schritte: Datenabnahme und
Einzelprojekt-Fehlerzustände, native Dokument-/Unicode-Semantik und Schrift-Fallback,
Datenvertrag, reale Sicherungsfehler, Leistung und dauerhafte Distribution abnehmen.
OS-Dialogbedienung, VoiceOver/NVDA/Orca, echte Systemsteuerungswechsel, individuelle
Windows-Kontrastfarben und Geräte-/Langzeitprüfungen sind noch nicht vollständig belegt.

Eigener Code: MIT. Apple-Developer-Konto und Windows-Signaturzertifikat fehlen.
1.0 bleibt bis zur ausdrücklichen Nutzerfreigabe gesperrt. Produkttexte werden
auf Nutzerwunsch erst abschließend bereinigt, wenn das Produkt vollständig ist.
Chat-Anbieter, Synchronisation und automatische KI-Pflege sind spätere Optionen.

## Versionsangaben 0.8.0

Versionskarte und CLI-/App-Option `--version` sind implementiert. 13 Kerntests
und erste Tastatur-/Versionsprüfungen bestehen. Das korrigierte Bild und Intel-Paket sind
geprüft: 126 Desktop-, 112 Tastatur- und 75 Sicherungs-UI-Aussagen sowie
Neustart und CLI-Sicherung. Die native Abnahme zu 1ab4ab5 besteht mit 20 Jobs und vier Paketen;
Windows-CRLF ist im Vergleich berücksichtigt.
Die dauerhafte Vorabversion v0.7.3 ist öffentlich: 22 erfolgreiche Jobs, vier
Archive und SHA256SUMS; öffentliche Downloads und API-Digests stimmen überein.
[Übergabe vom 7. Oktober](journal/2026-10-07.md).

## Textdarstellung 0.9.0

Neue geformte Textläufe und Noto-Ersatzschriften sind in C an die UI angebunden.
26 lokale Release-Tests, drei Nachprüfungen und das entpackte Intel-Paket
bestehen; 13 reine C-Kerntests ebenfalls. Der neue Texttest schlägt beim
bisherigen Renderer fehl. [Vertrag und Grenzen](../../docs/TEXTDARSTELLUNG.md):
gemischte Schreibrichtungen, graphemgenaue Eingabe, Emoji und native
Textgeometrie bleiben eigene Arbeiten. Native 0.9.0-Abnahme folgt.
