# SecondBrain: aktueller Stand

Stand: 7. Oktober 2026. Maßgebliche Originale:
[Umsetzungsstand](../../docs/STATUS.md), [Plattformnachweise](../../docs/PLATTFORMEN.md)
und [Release-Aufgaben](../../docs/RELEASE.md). Historische Schritte stehen im
[Journal](journal/2026-10-07.md).

## Produkt und geprüfter Stand

Eigene C17-Desktop-App für Mensch und KI: Projekte und Notizen anlegen,
lesen, bearbeiten, suchen und archivieren; Quellen lesen und gespeicherten
Projektkontext kopieren. Lumen-Sternkarte, eigene Glasdarstellung und Icons,
direkte Pfeilnavigation, Raumfahrt, begrenztes weiches Scrollen und große
Leseansicht sind implementiert. Entwürfe und erkannte Konflikte bleiben geschützt.
Eigene Inhaltsarchive bieten Sicherung, Prüfung und Wiederherstellung;
Einstellungen und Arbeitsstand bleiben gespeichert. [Verträge und Grenzen](SOURCES.md).

Die [0.9.5-Abnahme zu 37dba58](https://github.com/Lulus792/SecondBrain/actions/runs/37556151012)
besteht mit allen 20 Jobs einschließlich Windows Debug und vier entpackten
Paketen für Windows x64, Linux x64, macOS ARM64 und Intel. Gemeinsame eigene
C-Regeln verbinden Inline-Code, Maskierungen und Verweise in Titel, Leseansicht
und Sternkarte. Das lokal geprüfte Intel-Paket liegt unter dist/SecondBrain.
Öffentliche dauerhafte Vorabversion: [v0.7.3](https://github.com/Lulus792/SecondBrain/releases/tag/v0.7.3).

## Laufender Schritt 0.9.6

[Tabellen](../../docs/TABELLEN.md) sind in eigenem C-Code implementiert:
Spaltenausrichtung, gestapelte Zeilen bei schmaler Karte, erhaltene Originalbytes,
gemeinsame Verweise und Dokument→Tabelle→Zeile→Zelle-Struktur. 18 Kernprüfungen
bestehen; nach ergänzten Grenzfällen bestehen die drei Markdown-Nachprüfungen
und ASan/UBSan mit 5.644 Assertions. Der native macOS-Basisdurchlauf besteht.
32 erste Release-Tests und sechs abschließende Nachprüfungen mit 256 nativen
Assertions bestehen; Geometrie, Zellenansprung und Rasteransichten sind geprüft.
Das entpackte Intel-Paket einschließlich Desktop, Tastatur, Sicherung, Neustart
und CLI besteht; dist/SecondBrain enthält 0.9.6. Die frühere Betriebssystem-Startblockade ist behoben.
UIA GridPattern und AT-SPI Table fehlen in den festgelegten Providern; die
separate native Kinderprüfung ersetzt keine vollständige Tabellenbedienung.

## Weiterarbeiten

Die geprüfte Umsetzung ist als 648a696 gepusht. Die
[native 0.9.6-Abnahme](https://github.com/Lulus792/SecondBrain/actions/runs/37593650781)
läuft noch; tatsächliche Ergebnisse übernehmen und offene Fehler beheben.
Danach verbleibende Container-/Inline-/Listenregeln,
Unicode-Textgeometrie/IME, Support-/Lizenzzuordnung und übrige Release-Aufgaben.
Tatsächliche native Dialog- und menschliche VoiceOver/NVDA/Orca-Bedienung,
Geräte-/Langzeitprüfungen und weitere volle Dateisysteme bleiben offen.
CI-Nachweise belegen keine vollständige menschliche oder physische Geräteabnahme.

Der vollständige Auftrag vor 1.0 bleibt aktiv. Eigener Code: MIT.
Apple-Developer-Konto und Windows-Signaturzertifikat fehlen.
1.0 wird ausschließlich nach ausdrücklicher Nutzerfreigabe gesetzt.
Die abschließende Bereinigung der Produkttexte folgt erst, wenn das Produkt
vollständig ist. Originalnachweise bleiben erhalten.
