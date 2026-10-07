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

## Aktueller Arbeitsschritt 0.9.10

[Abschnittstrennungen](../../docs/TRENNLINIEN.md), skalierte Reader-Innenabstände
und ein sichtbarer Hilfe-Punkt sind umgesetzt. Nach Paddingkorrektur bestehen
alle 33 lokalen Intel/macOS-Release-Tests (195,44 s), darunter 342 native Assertions.
181.838 Blockassertions und eigene ASan/UBSan-Instrumentierung bestehen; vorheriger
Blockleser scheitert. Das entpackte erste Paket besteht mit Desktop 126, Tastatur
142, Sicherung 75, Neustart und CLI. Das korrigierte Paket besteht ebenfalls mit allen genannten Paketwegen;
dist enthält 0.9.10, Build f3a758eb61d9.

Erste neue Windows-/Linux-CI scheitert am Kontrastpixel. Ein tatsächliches lokales
1x-Fenster reproduziert Grau statt des vorgesehenen deckenden Kerns. Die Linie
hat im Kontrastmodus jetzt ausreichende integrale Stärke. Nachprüfung mit 352
Assertions besteht bei 1x/2x-Pixeldichte und 100/200 Prozent Schriftgröße; strenge
Farbanforderung bleibt erhalten. Die neue native CI 37613957814 läuft; tatsächliche
Windows-/Linux- und Mac-Ergebnisse übernehmen.
UI-Bericht: ../../../UI_reviewer/reviews/secondbrain/2026-10-07_13-01-27/UI_REVIEW.md.

0.9.9 zu 099de24 besteht unter Windows/Linux einschließlich Debug/Release und
Paketen; Mac-Jobs stehen noch aus. Ein früherer einzelner Windows-Debug-Kernfehler
hat keine bekannte Ursache, obwohl die neuen Kernläufe bestehen. Diagnose und
Log-Artefakte sind nun auch im Kernworkflow verfügbar.

## Nächste Arbeit und Grenzen

Neue native Kontrastnachprüfung übernehmen. Danach verbleibende Container-/
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
