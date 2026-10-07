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

## Geprüfter lokaler Schritt 0.9.6

[Tabellen](../../docs/TABELLEN.md) sind in eigenem C-Code implementiert:
Spaltenausrichtung, gestapelte Zeilen bei schmaler Karte, erhaltene Originalbytes,
gemeinsame Verweise und Dokument→Tabelle→Zeile→Zelle-Struktur. 18 Kernprüfungen
bestehen; nach ergänzten Grenzfällen bestehen die drei Markdown-Nachprüfungen
und ASan/UBSan mit 5.644 Assertions. Der native macOS-Basisdurchlauf besteht.
32 erste Release-Tests und sechs abschließende Nachprüfungen mit 256 nativen
Assertions bestehen; Geometrie, Zellenansprung und Rasteransichten sind geprüft.
Das entpackte Intel-Paket einschließlich Desktop, Tastatur, Sicherung, Neustart
und CLI besteht; dist/SecondBrain enthält inzwischen das geprüfte 0.9.7-Paket. Die frühere Betriebssystem-Startblockade ist behoben.
UIA GridPattern und AT-SPI Table fehlen in den festgelegten Providern; die
separate native Kinderprüfung ersetzt keine vollständige Tabellenbedienung.

## Geprüfter lokaler Schritt 0.9.8

Die [0.9.7-Abnahme](https://github.com/Lulus792/SecondBrain/actions/runs/37596496154)
besteht jetzt mit allen 20 Jobs und vier Paketen. 0.9.8 ist als 605527e gepusht:
eigene C-Grapheme nach Unicode 18.0, ganze Zeichen in Eingaben, atomischer Undo-
Vorgang, native Teilmarkierungen und vollständige geformte Editorzeilen. 19 Kern-
prüfungen, 33 erste Release-Tests und sechs abschließende Prüfungen bestehen;
853 Normfälle, 111 Editor- und 273 native Assertions sind geprüft. Eigene Bereiche
werden unter ASan/UBSan geprüft; vorherige Navigation scheitert am Akzentfall.
Ein früherer Paketlauf scheitert an gewechselter Zwischenablage im Pfadfixture;
Testeingaben verwenden jetzt UTF-8-sichere SDL-Abschnitte; Kopierwerte werden
sofort nach der Aktion geprüft. Desktop 126 und Tastatur 142 bestehen nochmals.
Das neu erzeugte Intel-Paket einschließlich Desktop, Tastatur, Sicherung,
Einstellungsneustart und CLI besteht; dist/SecondBrain enthält 0.9.8.
Die [native 0.9.8-Abnahme](https://github.com/Lulus792/SecondBrain/actions/runs/37604300744)
läuft noch; tatsächliche Ergebnisse übernehmen und neue Fehler beheben.
[Vertrag](../../docs/GRAPHEME.md), [Nachweise](../../docs/STATUS.md).

## Schritt 0.9.9

[Emoji-Schrift und Schriftläufe](../../docs/EMOJI.md) sind implementiert.
33 erste lokale Release-Tests und drei abschließende Raster-/Lizenzprüfungen
bestehen mit 43 Text-, 111 Editor- und 58 Lizenzassertions. Eigene Text-/Graphem-
Instrumentierung besteht; vorheriger reiner Fallback zerlegt die Emoji-Verbindung.
Das neue Intel-Paket besteht mit Desktop 126, Tastatur 142, Sicherung 75,
Neustart und CLI; dist enthält 0.9.9, Build cf5678a1a177. Erste native Windows-/
Linux-Release-Tests finden eine um einen Pixel zu enge Familienbreiten-Assertion.
Korrektur plus zusätzlicher Zerlegungsprüfung besteht lokal mit 46 Assertions
und ASan/UBSan; neue native Abnahme folgt nach Push. Keine vollständige neue
Plattformabnahme behaupten. [Originalnachweise](../../docs/STATUS.md).

Support-/Beitrags-/Issue-Vorlagen und Abhängigkeitsupdate-Ablauf sind als 099de24
gepusht; YAML und lokale Verweise geprüft. GitHub verlangt für die tatsächliche
Formularansicht eine Anmeldung; diese Darstellung ist noch nicht abgenommen. Vertraulicher Sicherheitskanal ist angefragt.
Der Nachlauf zu 099de24 besteht unter Windows/Linux einschließlich Debug,
Release und Paketen. Der frühere Windows-Debug-Kernfehler wiederholt sich dort
nicht; Ursache bleibt ungeklärt. Vorhandene CTest-Diagnose und Log-Artefakte
sind jetzt auch im Kernworkflow angebunden. Mac-Jobs stehen noch aus.

## Laufender Schritt 0.9.10

[Abschnittstrennungen](../../docs/TRENNLINIEN.md), skalierte Reader-Innenabstände
und der Hilfe-Punkt sind implementiert und im Raster nachgeprüft. Abschließend
bestehen alle 33 lokalen Release-Tests (195,44 s), einschließlich nativer Rolle,
echter Kontrastpixel und Abstand zum Fokusring. Eigene Block-/Test-Instrumentierung
besteht unter ASan/UBSan; alte Blockerkennung scheitert. Neues Paket und native
Plattformabnahme folgen; dist bleibt bis zur bestandenen Paketprüfung bei 0.9.9.

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
