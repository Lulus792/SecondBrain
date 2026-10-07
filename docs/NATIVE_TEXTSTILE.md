# Textstile in nativen Bedienhilfen

Stand: 7. Oktober 2026, Entwicklungsschritt 0.9.12.
Vor der Anbindung eingesehen: die festgelegten Quellen von AccessKit C 0.23.1,
macOS 0.27.1, Windows 0.35.1, Unix 0.24.0 und AT-SPI Common 0.21.0.
Die bereits dokumentierte Apple-Typografie-Recherche ist Gestaltungsgrundlage.
[AppKit-Attributabfrage](https://developer.apple.com/documentation/appkit/nsaccessibilityprotocol/accessibilityattributedstring(for:))
liefert formatierten Text für einen Zeichenbereich. Die native Linux-Prüfung
verwendet [AT-SPI Textattribute](https://gnome.pages.gitlab.gnome.org/at-spi2-core/libatspi/method.Text.get_text_attributes.html).
Die lokale Originalquellenprüfung ergänzt nicht erreichbare docs.rs-Seiten;
keine ungelesene Webantwort wird als Nachweis verwendet.

## Gemeinsame Daten

Absätze, formatierte Überschriften, Codeblöcke und Tabellenzellen veröffentlichen
Textläufe mit ihren Stilbereichen. Dieselbe Graphem-Anpassung wie beim Zeichnen
bestimmt die Bereiche: Ein Graphem übernimmt den Stil seines ersten Skalarwerts.
Kerntext und gespeicherte Originalbytes bleiben erhalten. Passive UI-Daten und
der native Snapshot besitzen ihre eigenen Kopien; Textläufe haben stabile IDs
innerhalb des Dokumentkontexts. Nur geänderte Formatierung aktualisiert ebenfalls
den Snapshot. Der Quelleditor bleibt ein zusammenhängender, unformatierter
Textlauf mit seinem bisherigen Auswahl-/Bearbeitungsvertrag.

Die veröffentlichte Schriftrolle enthält Größe in logischen Punkten, Gewicht
400/700, Kursivmerkmal und die primäre Familie Noto Sans beziehungsweise
Noto Sans Mono. Fett/Kursiv entsprechen den synthetischen UI-Stilen. Eine Familie
beschreibt die primäre Schriftrolle; damit wird nicht die tatsächliche Ersatzschrift
jedes einzelnen Zeichens zugesagt. Präzise Fallback-/Zeichenrechtecke bleiben
Teil der offenen Textgeometriearbeit. Die Rechtecke der Textläufe behalten bisher
die grobe Block-/Zellgeometrie und sind keine exakten Zeichenrechtecke.

## Plattformen

AccessKit setzt macOS-Textbereiche in Attributed Strings mit den AppKit-
Fontattributen um. Windows stellt Gewicht, Kursiv, Name und Größe über UIA-
Textbereiche bereit. Linux verwendet AT-SPI-Attribute `weight`, `style`,
`family-name` und `size`; ein fehlendes Kursivmerkmal bedeutet hier normale
Schrift. Adapterquellen belegen diese Umsetzung; erfolgreiche native Aufrufe
werden getrennt anhand der tatsächlich ausgeführten Plattformtests dokumentiert.

## Prüfung und Grenzen

Die native Prüfung liest Attribute für normalen, kursiven, fetten, kombiniert
hervorgehobenen und Mono-Text. Formatierte Überschrift und Codeblock haben ihre
jeweiligen Größen. Unicode vor den Prüfwörtern prüft die Indexumsetzung. Ein
Stiltausch bei gleichem Klartext muss sichtbar werden, Code bei 200 Prozent
muss seine skalierte Größe veröffentlichen. Eine zusätzliche API-Fixture ändert
ausschließlich den Stil bei identischem Text, Titel, Rolle und Rechteck; echte
native Abfragen prüfen diese Aktualisierung unabhängig von Quelltextänderungen. Weitere bestehende Abfragen prüfen
vollständigen Dokumenttext, Tabelle, Auswahl und Schreibschutz weiter.

Menschliche VoiceOver/NVDA/Orca-Bedienung, alle assistiven Darstellungspräferenzen,
Bidi, präzise visuelle/native Textgeometrie und IME bleiben separate Release-
Abnahmen. Native Attribute allein schließen diese Aufgaben nicht ab. Tatsächliche
Ergebnisse: [Umsetzungsstand](STATUS.md) und [Plattformen](PLATTFORMEN.md).
