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

## Aktueller Arbeitsschritt 0.9.14

[Zeichenreferenzen](../../docs/ENTITIES.md) sind im eigenen C-Leser implementiert:
2.125 festgelegte Namen, numerische Angaben, stabile Ausgabestile, erhaltene
Literalbereiche und Quellen, dekodierte lokale Ziele und richtige Sternkarten-
beziehungen. Math-Fallback ersetzt bestätigte Kästchen; Originaldaten/-fonts
und Lizenzen sind zugeordnet. 36 Release-Tests, abschließende Kern-/native
Nachprüfungen und neun Python-Strukturtests bestehen lokal. Eigene Kern-
ASan/UBSan mit 1.194.343 Assertions besteht; letzte native Sanitizer-Nachprüfung
besteht mit 393 Assertions. UI-Bericht SBUI-048/049. Neue Paket-/Plattformabnahme folgt.
Dist enthält bis dahin geprüftes 0.9.13, Build cade9ea34b95.

Vorherige Windows-/Linux-Abnahme 37631991014 zu 9c38604 besteht in zwölf Jobs,
einschließlich Debug/Release und Release-Paketen. Neue Mac-CI separat übernehmen;
keinen Plattformnachweis der neuen 0.9.14 daraus ableiten.

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
