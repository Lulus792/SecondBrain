# Hervorhebungen und Inline-Code

Stand: 7. Oktober 2026, Entwicklungsversion 0.9.11.
Grundlagen vor der Anbindung: [CommonMark 0.31.2](https://spec.commonmark.org/0.31.2/#emphasis-and-strong-emphasis),
[Referenzalgorithmus](https://github.com/commonmark/commonmark.js/blob/0.31.2/lib/inlines.js)
und [SDL_ttf-Schriftstile](https://wiki.libsdl.org/SDL3_ttf/TTF_SetFontStyle).
Die vorhandene [Apple-Typografie-Recherche](TEXTDARSTELLUNG.md) bleibt die
Gestaltungsgrundlage. Die Auswahl und technische Umsetzung sind eigene Entscheidungen.

## Gemeinsame C-Verarbeitung

Der eigene Inline-Leser erzeugt Text und Stilbereiche für kursiv, fett,
kombinierte Hervorhebungen und Inline-Code. Titel, Linkbeschriftung und Leseansicht
verwenden dieselben Regeln. Passende Stern-/Unterstrichfolgen werden anhand ihrer
Nachbarn und der Dreierregel zugeordnet; ungepaarte Marker bleiben Text.
Unicode-Zeichengruppen stammen aus festgelegten Unicode-18-Daten. Unterstriche
innerhalb normaler Wörter erzeugen keine Hervorhebung. Code und Maskierungen
bleiben eigenständige Bereiche. Linkbeschriftungen begrenzen innere Paarungen.

Winkel-URLs werden als Autolinks erkannt; rohe HTML-Elemente bleiben vollständig
als Literaltext sichtbar und werden nicht ausgeführt. Ihre Attribute erzeugen
keine Hervorhebungen oder lokalen Verweise. Leere Listenmarker und geordnete
Marker mit anderem Start als 1 unterbrechen keinen bestehenden Absatz.
Weitere Container- und Referenz-/Entity-Regeln bleiben eigene Arbeiten.

Originaltext wird nicht geändert. Der Kern verwendet weiterhin nur C und
Standard-/Betriebssystemfunktionen. Unicode-Eigenschaftsdaten sind lizenzierte
Daten; es kommt keine fachliche externe Bibliothek hinzu.

## Darstellung

SDL_ttf erzeugt bei Bedarf getrennte Schriftkopien für Stile. Die Kopien teilen
die Schriftdatei; Originalassets bleiben unverändert. Fett und kursiv sind
synthetische SDL_ttf-Stile. Inline-Code verwendet die Mono-Schrift in der Größe
der umgebenden Schriftrolle, auch in Überschriften. Die Leseansicht und Tabellen
verwenden dieselben Stilbereiche und dieselbe Mess-/Layoutfunktion.

Schriftmetriken geben den Fragmenten eine gemeinsame Grundlinie. Stilbereiche
werden zum Zeichnen an vollständige Grapheme angepasst: Ein Graphem übernimmt
den Stil seines ersten Skalarwerts. Das verändert weder die logischen Stilbereiche
noch Quelldateien. Lange Wörter werden in begrenzten Textstücken gemessen;
Umbrüche teilen keine Grapheme. Eine überbreite einzelne Kombination wird als
Einheit geometrisch geclippt. Formatierte erste Überschriften bleiben im Inhalt
sichtbar und verlieren ihre Hervorhebung nicht durch den schlichten Kartenkopf.

## Prüfung und Grenzen

Die eigenen Kernfälle und 5.000 begrenzte Eingaben prüfen Text, Stile und erhaltene
Originalbytes. Die 132 ausgewählten offiziellen Hervorhebungsfälle sind mit der
C-Probe gegen die erwartete Text-/Stilstruktur geprüft. Drei rohe HTML-Fälle
verwenden ausdrücklich den Literal-Anzeigevertrag; daraus wird keine vollständige
CommonMark-/HTML-Konformität abgeleitet. [Herkunft und Datenlizenz](../tests/data/commonmark-0.31.2/README.md).

Die Textprüfung verlangt tatsächliche verschiedene Schriftkommandos, passende
Codegröße in Überschriften, gleiche lateinische/arabische Grundlinien und
vollständige Akzent-/Emoji-Kommandos trotz Formatgrenzen bei drei Schriftgrößen.
Ausgeführte abschließende Ergebnisse stehen in STATUS.md.

Gemischte Bidi-Absätze, kontextuelle Script-Formung über Schriftstilgrenzen,
präzise Zeichenrechtecke und menschliche assistive
Bedienung bleiben offen. Die neue visuelle Formatierung ersetzt diese Abnahmen nicht. Ab 0.9.12
werden [native Textstile](NATIVE_TEXTSTILE.md) separat angebunden und geprüft.
