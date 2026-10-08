# Native Zeichen und Wortanfänge

Stand: 8. Oktober 2026, Umsetzung ab 0.9.41.

Native Textläufe sind auf 255 auswählbare Einheiten begrenzt. AccessKit verwendet
für Zeichenlängen und Wortindizes acht Bit; ein unbeschränkt langer Lauf darf
keine Wortindizes darin ablegen. Normale Grapheme einschließlich kombinierter
Akzente und Emoji bilden jeweils eine Einheit. CRLF zählt ebenfalls als eine
Einheit. Harte Zeilenenden und Stilwechsel beginnen einen neuen Lauf; nach einem
abschließenden Zeilenende bleibt ein leerer Schlusslauf für die Cursorposition.
Die zusammengefügten Laufwerte erhalten den gesamten ursprünglichen UTF-8-Text.

Wortanfänge entstehen aus denselben Unicode-Regeln wie die UI. Die Berechnung
verwendet den ganzen Wert, damit ein langer oder an einer Stilgrenze geteilter
Wortkörper keinen erfundenen neuen Wortanfang erhält. Wortstarts liegen auf
Graphemgrenzen. Laufidentitäten hängen am Quellenoffset und bleiben bei
unverändertem Text und veränderter Position erhalten.

Native Auswahlpositionen zählen lauflokale Einheiten. Der Adapter übersetzt sie
in globale Unicode-Skalarpositionen für den vorhandenen Editor. Anker und Ende
dürfen aus unterschiedlichen Läufen desselben Eingabeelements stammen. Fremde
Laufkennungen und ungültige Indizes werden abgewiesen. Die Übersetzung und das
Einreihen der Aktion erfolgen unter derselben Sperre. Eine veränderte Eingabe
invalidiert bereits eingereihte Aktionen mit alter Positionszuordnung.

Unveränderte Texte und Stilquellen verwenden bei Geometrieänderungen die
vorhandenen Metadaten erneut. Ein strukturiertes Lesedokument benötigt keinen
zusätzlichen Metadatenplan seines rohen Markdown-Werts; seine Inhaltsblöcke
tragen die sichtbare Textstruktur.

## Grenzen und Abnahme

Die native Abbildung eines Graphems mit mehr als 255 UTF-8-Bytes passt nicht in
diesen Bibliotheksvertrag. Solche ungewöhnlich langen Kombinationen erhalten
skalare Teilstücke, damit kein Quellenbyte verloren geht. Die UI klemmt ihre
Auswahl weiterhin auf ganze Grapheme. Das ist eine benannte native Grenze und
kein Nachweis identischer Zeichenschritte für diese Sonderfälle.

Native Wortenden können über den jeweiligen Plattformvertrag nachfolgende
Leerzeichen einschließen. Die Metadaten vereinheitlichen die tatsächlichen
Wortanfänge; sie behaupten keine sprachabhängig identischen Wortbefehle aller
Screenreader. Wörterbuchanalyse bleibt außerhalb dieses Umfangs.

Zeichenpositionen, Zeichenbreiten und genaue Glyphenrechtecke sind noch nicht
angebunden. Die Laufrechtecke verwenden vorerst die vorhandene Elementfläche.
Die hier geprüfte Auswahl- und Wortabbildung ist die Grundlage der weiteren
Geometriearbeit, kein abgeschlossener Screenreader-/Vergrößerungstest.

Primärvertrag: AccessKit 0.25.1, `Node::character_lengths`, `word_starts` und
`character_positions`, lokale festgelegte Bibliotheksquelle am 8. Oktober gelesen:
[Quellcode](https://github.com/AccessKit/accesskit/blob/ce8164ba92995cfa86005b6259115e08c8244253/accesskit/src/lib.rs).
Die Wortregeln und UI-Konventionen stehen in [WORTNAVIGATION](WORTNAVIGATION.md).
Konkrete Tests und tatsächliche Plattformnachweise stehen in [STATUS](STATUS.md).
