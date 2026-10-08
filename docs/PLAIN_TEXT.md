# Gemeinsame Geometrie für einfache UI-Texte

Stand: 8. Oktober 2026. Entwicklungsschritt 0.9.36; Nachweise stehen in
[STATUS](STATUS.md). Der bisher getrennte Mess-/Rasterweg für einfache Texte
wird durch denselben [Glyphenvertrag](GLYPHENGEOMETRIE.md) wie in Reader und
Editor ersetzt.

## Umfang

Die Fontbreitenfunktion und der produktive Textkommandokonverter verwenden jetzt
Absatzrichtung, Scriptauflösung, ganze Grapheme, Ersatzschriften und tatsächliche
Glyphenpositionen. Das betrifft unter anderem Buttontexte, Projektnamen,
Sternbeschriftungen, Hinweise und einfache Textkommandos. Arabische Verbindungen,
Ligaturen und gemischte Richtungen werden im Zeilenkontext geformt; der Quelltext
bleibt in seiner logischen Reihenfolge. Messung benutzt Glyphenfortschritte statt
summierter, gerundeter Breiten separat gerasterter Schriftteile.

[app/plain_text.inc](../app/plain_text.inc) hält exakte Schlüssel aus Textbytes
und Fontrolle. Maximal 128 Pläne und 16 MiB bleiben gespeichert. Größere Pläne
werden vorübergehend vorbereitet und nach dem Aufruf freigegeben; das begrenzt
nicht deren Spitzenbedarf. Font-/Dichtewechsel zerstören Pläne vor den Fonts.
Die Rastertexturen teilen den vorhandenen Glyphencache mit Reader und Editor;
sie bleiben bis nach der Frameausgabe gültig.

Sehr breite Zeilen werden vollständig geformt und nur in sichtbaren Ausschnitten
gezeichnet. Ein ausgeschnittener Bereich wird nicht neu geformt. Float-Positionen
bleiben erhalten. Die eigenständige Prüfung einer über 65.535 logische Einheiten
breiten Zeile vergleicht ihren sichtbaren Abschluss mit einer kürzeren Referenz.

Das Kürzen sucht über ganze Graphemgrenzen mit tatsächlich geformten Präfixbreiten.
Ein eigener optionaler Nuklear-Hook verwendet diese Messung auch vor dem Anlegen
von Textkommandos. Wortumbruch bevorzugt weiterhin logische Leerzeichengrenzen;
bei zu schmaler Umbruchfläche wird wenigstens eine ganze Einheit verbraucht.
Ein gekürztes einzelnes Label kann leer bleiben, wenn keine Einheit hineinpasst.
Die Binärsuche setzt zunehmende Präfixfortschritte voraus; eine normative
Abnahme exotischer Fonts mit nichtmonotonen Präfixfortschritten steht aus.

## Verbleibende Arbeiten

Ab 0.9.37 verwendet auch der [allgemeine Widgetumbruch](WRAPPED_TEXT.md) einen
zusammenhängenden Absatzplan für Höhe und Rasterung. Dessen Nachweise und
weiteren Grenzen stehen im eigenen Vertrag; der hier dokumentierte Schritt
0.9.36 allein belegt diese spätere Integration nicht.
Native Zeichenrechtecke, reale Eingabemethoden, Unicode-Wortnavigation im Editor,
Screenreader und große reale Daten bleiben eigene Aufgaben.

## Prüfung

Eine unabhängig festgelegte Font-/Scriptaufteilung formt eine Referenz aus
Latein, Ligaturen/Akzenten, Hebräisch mit Ziffern, Arabisch und Emoji. Messung
wird mit deren Glyphenfortschritt verglichen; der produktive Textkonverter wird
bei drei Schriftgrößen und links/rechts/mittig gesetzten Positionen pixelgenau
mit der Referenz verglichen. Wiederholte Frames prüfen den Cache. Ein zusätzlicher
Latein-/Griechischfall prüft Scriptwechsel innerhalb derselben Schrift. Kürzung
von Akzenten, ZWJ-Emoji, Flaggen und indischen Verbindungen darf keine Teilgrapheme
anlegen. Die vorhandene lange Zeilen-, Reader-, Editor- und Navigationsprüfung
bleibt Bestandteil der Gesamtprüfung. Ausgeführte Ergebnisse folgen in STATUS.
