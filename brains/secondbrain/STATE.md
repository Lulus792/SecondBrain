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

## Aktueller Arbeitsschritt 0.9.13

[Windows-UIA-Textsuche](../../docs/UIA_TEXTSUCHE.md) ist tatsächlich nachgeprüft:
Lauf 37631991014 zu 9c38604 besteht in allen zwölf Windows-/Linux-Jobs, einschließlich
Debug/Release, nativen Stil-/FindText-Abfragen und entpackten Release-Paketen.
Native Windows-Konfiguration erzwingt LF-Bytes; die UI-DLL verwendet den
geprüften Patchhash 453fcaaa4faf52c87cc8f15fc50d4710540e35bc181bd6ac3daf28613ed7874a.
Vier Quellenfälle und Fremdquellen-Abweisung bestehen lokal. UI-Bericht SBUI-047
und vorherige Stilabnahme SBUI-046 sind im beschriebenen Windows-/Linux-Umfang
nachgeprüft. Neue Mac-CI-Jobs warten noch; lokal 380 native, 85 Text- und 111
Editorassertions sowie vier Nachprüfungen bestanden.

Das abschließende Intel-Paket besteht mit Desktop 126, Tastatur 142, Sicherung
75, Neustart und CLI. Dist enthält 0.9.13, Build cade9ea34b95; eigene Instanz
geladen und Raster betrachtet, vorherige App erhalten. Die nächste C-Parserarbeit
ist in [Zeichenreferenzen-Recherche](../../docs/ENTITIES_RECHERCHE.md) vorbereitet,
noch nicht implementiert. Menschliche assistive und Geometrieabnahmen bleiben offen.

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
