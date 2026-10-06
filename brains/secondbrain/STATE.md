# SecondBrain: aktueller Stand

Stand: 6. Oktober 2026. Originale: [Umsetzungsstand](../../docs/STATUS.md),
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
  sind gezielt nachgeprüft. dist/SecondBrain ist lokal auf 0.7.0.

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
Die Regression schlägt beim bisherigen Kern fehl; Plattform-/Paketabnahme folgt.
[Datenvertrag](../../docs/DATENVERTRAG.md).

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
