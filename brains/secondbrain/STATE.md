# SecondBrain: aktueller Stand

Stand: 7. Oktober 2026. Maßgeblich sind [STATUS](../../docs/STATUS.md),
[Plattformnachweise](../../docs/PLATTFORMEN.md) und [Release-Aufgaben](../../docs/RELEASE.md).
Details vergangener Schritte bleiben im [Journal](journal/2026-10-07.md).

## Produkt

Eigene C17-Desktop-App für Mensch und KI: Projekte/Notizen verwalten, Wissen lesen,
bearbeiten, suchen und archivieren; Quellen lesen und gespeicherten KI-Kontext
kopieren. Lumen-Sternkarte, eigene Glaskarten und Icons, direkte Pfeilnavigation,
Raumfahrt, begrenztes weiches Scrollen und große Leseansicht sind implementiert.
Entwürfe und erkannte Konflikte bleiben geschützt. Sicherung/Wiederherstellung,
Einstellungen, eigener Markdown-Blockleser und vollständige Graphem-Eingaben
gehören zum bisherigen geprüften Umfang. [Verträge und Quellen](SOURCES.md).

## Aktueller Arbeitsschritt 0.9.11

[Gemeinsame Inline-Stile](../../docs/INLINE_STILE.md) sind umgesetzt: sichtbare
Hervorhebungen, Mono-Code, gemeinsame Grundlinien und vollständige Grapheme über
Stilgrenzen. 34 erste Release-Tests und sieben abschließende betroffene Prüfungen
bestehen lokal; 85 Text-, 352 native Assertions sowie 132 ausgewählte Normfälle.
Eigene ASan/UBSan und neun Python-Strukturtests bestehen. Drei Raster betrachtet.
Genauen Umfang und Grenzen nennen STATUS und der UI-Bericht SBUI-044/045.
Das neue Intel-Paket besteht mit Desktop 126, Tastatur 142, Sicherung 75,
Neustart und CLI. dist enthält 0.9.11, Build 54aa29fd7bc8; eigenes Gedächtnis
geladen und Raster betrachtet. Neue Abnahme 37621658743 läuft: alle Kern-/
Python-Jobs bestehen, Desktopjobs laufen oder warten noch.

Die vorherige 0.9.10-Abnahme 37613957814 ist vollständig erfolgreich: 20 Jobs
auf Windows, Linux, Intel-/ARM64-macOS einschließlich Release-Paketen. Kein neuer
Plattformnachweis für 0.9.11 daraus ableiten.

## Nächste Arbeit und Grenzen

Neue Paket-/Plattformnachweise übernehmen. Danach verbleibende Container-/
Inline-/Listenregeln, Bidi, visuelle/native Textgeometrie und IME, native Tabellen-
Matrixschnittstellen sowie übrige Release-Aufgaben umsetzen und abnehmen.
Support-/Beitrags-/Issue-Vorlagen und Updateablauf sind veröffentlicht; vertraulicher
Sicherheitskanal ist angefragt, transitive Lizenzprüfung und Wartungsverantwortung
bleiben offen. GitHub-Formularansicht verlangt eine Anmeldung und ist noch nicht abgenommen.

Menschliche VoiceOver/NVDA/Orca-, native Dialog-, Geräte-/Langzeit- und weitere
Vollvolume-Abnahmen bleiben offen. CI und Raster ersetzen diese Nachweise nicht.
Der vollständige Auftrag bleibt aktiv. Eigener Code: MIT; Signaturkonten fehlen.
1.0 und abschließende Produkttext-Bereinigung erst nach den dafür festgelegten
Voraussetzungen beziehungsweise ausdrücklicher Nutzerfreigabe.
