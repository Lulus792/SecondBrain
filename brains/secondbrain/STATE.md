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

## Aktueller Arbeitsschritt 0.9.12

[Native Textstile](../../docs/NATIVE_TEXTSTILE.md) sind angebunden: gemeinsame
Stilbereiche, primäre Fontrollen und Aktualisierung bei gleichem Klartext.
34 lokale Release-Tests bestehen (279,40 s); abschließend 380 native Assertions,
zusätzlich 85 Text- und 111 Editorassertions. Fonttraits, Überschrift/Codeblock,
Tabellenzelle und 200-Prozent-Codegröße sind über echte AppKit-Abfragen geprüft.
Voriger Provider scheitert; erste eigene ASan/UBSan mit 376 Assertions besteht.
Abschließende eigene ASan/UBSan mit 380 Assertions besteht; ohne Stilsignatur
scheitert die reine Stil-Fixture. UI-Bericht SBUI-046.
Neue Paket-/Plattformabnahme folgt; dist enthält bisher 0.9.11, Build 54aa29fd7bc8.

Die vorherige 0.9.11-Abnahme 37621658743 ist vollständig erfolgreich: 20 Jobs
auf Windows, Linux, Intel-/ARM64-macOS einschließlich Release-Paketen. Daraus
keinen neuen Plattformnachweis für 0.9.12 ableiten.

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
