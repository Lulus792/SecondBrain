# Zusammenhängender Umbruch für einfache Texte

Stand: 8. Oktober 2026, Entwicklungsschritt 0.9.37. Der allgemeine Umbruch
verwendet jetzt denselben [Absatz-/Glyphenvertrag](GLYPHENGEOMETRIE.md) wie die
formatierte Leseansicht. Ausgeführte Prüfungen stehen in [STATUS](STATUS.md).

## Grundlage

Apple [Typography](https://developer.apple.com/design/human-interface-guidelines/typography)
und [Layout](https://developer.apple.com/design/human-interface-guidelines/layout)
wurden am 8. Oktober erneut über ihre offiziellen DocC-Daten gelesen.
Typografie soll bei angepassten Größen lesbar bleiben und ihre Hierarchie
behalten. Layouts sollen den verfügbaren Platz und größere Schrift berücksichtigen;
Textflächen brauchen gegebenenfalls mehr Höhe, damit Inhalte nicht überlappen
oder unbeabsichtigt abgeschnitten werden. Die konkrete C-Geometrie, Umbruch-
und Cachepolitik sind eigene Umsetzung, keine Apple-Komponenten.

## Gemeinsamer Plan

`nk_text_wrap` und `nk_label_wrap` verwenden über einen eigenen optionalen
Nuklear-Hook einen zusammenhängenden Absatzplan. `sb_ui_wrap_height` und
`sb_ui_text_aligned` benutzen denselben Plan für Höhe und Darstellung. Das gilt
auch für unformatierte Codeblöcke: ihre Höhe wird nicht mehr aus Gesamtbreite
und einer angenommenen Zeilenzahl geschätzt. Code bleibt wörtlicher Text;
der Weg interpretiert seine Markdown-Zeichen nicht als Formatierung.

Die Analyse wird vor dem Umbruch für den ganzen Absatz ausgeführt. Alle
Folgezeilen erhalten dessen Richtungs- und Schriftkontext. Ein hebräisch
beginnender Absatz behält seine Grundrichtung auch dann, wenn die nächste
Zeile nur lateinische Wörter enthält. Tatsächliche Glyphenfortschritte bestimmen
den Umbruch; Kandidaten werden im vollständigen Zeilenkontext nachgeprüft.
Ligaturen, arabische Verbindungen und Ersatzschriftmetriken bleiben erhalten.

Die bestehende Umbruchpolitik bevorzugt Leerzeichen; lange Wörter brechen
an ganzen Graphemgrenzen. Das ist weiterhin kein vollständiger Unicode-
Zeilenbruchalgorithmus nach UAX #14. Eine Einheit, die breiter als die verfügbare
Fläche ist, wird als Ganzes verbraucht und beim Zeichnen an der Fläche geclippt.

Explizite Absatztrenner und CRLF werden vollständig verbraucht. Leere Absätze
bekommen eine eigene Zeilenhöhe; ein abschließender Trenner erhält in einfachen
Textflächen auch die leere Abschlusszeile. Die formatierte Markdown-Ansicht
behält ihre bisherige Abschlussregel. Nicht nullterminierte Quellausschnitte
werden längenbegrenzt kopiert; der Originaltext wird nicht verändert.

## Maße, Clips und Cache

Einfache Umbruchflächen verwenden den vom Widget übergebenen Innenabstand und
zweimal dessen vertikalen Abstand als Zeilenabstand. Die Reader-Regel bleibt
bei vier skalierten Einheiten. Beide berücksichtigen die größeren Metriken der
verwendeten Ersatzschriften. Alle Zeilenhöhen plus obere/untere Innenabstände
bilden dieselbe gemessene Gesamthöhe, mit der die Fläche angelegt wird.

Text wird innerhalb des tatsächlichen Widgetrechtecks und des bisherigen
Fensterclips gezeichnet. Der vorherige Clip wird danach wiederhergestellt.
Links-, Mittel- und Rechtsausrichtung sind explizite physische Ausrichtungen.
Ein Textur-/Layoutfehler meldet einen Fehler; er zeichnet keinen zweiten,
anders umgebrochenen Text über schon angelegte Glyphen.

Der vorhandene 32-MiB-Layoutcache enthält zusätzlich Zeilenabstand und
Abschlussregel im exakten Schlüssel. Gleiche Bytes, Fonts und Breiten dürfen
zwischen Reader und einfacher Textfläche unterschiedliche Pläne haben.
Schrift-/Dichtewechsel verwerfen sie vor dem Schließen der Fonts.
Große Texte über 65.536 Bytes werden weiterhin frisch vorbereitet; deren
Speicherbedarf und flüssige Bearbeitung sind nicht mit diesem Schritt abgenommen.

`sb_ui_wrap_geometry` liefert geliehene Glyphenzeilen, Quelloffsets und vertikale
Positionen ausschließlich während eines Callbacks. Glyphenmaße bleiben in
Backing-Pixeln, y in lokalen Fensterpunkten. Der Callback darf den Font-/Layoutcache
nicht verändern. Die Rasterfunktion kann an den tatsächlichen Command-Buffer
zeichnen, auch wenn ein Widget seinen eigenen Buffer übergibt.

## Prüfung und Restumfang

Eine unabhängige Referenz probiert jeden Graphemendpunkt gegen den ursprünglichen
Absatz aus. Ihre Font-/Scriptaufteilung ist separat festgelegt. Vergleichsfälle
umfassen RTL mit lateinischen Folgezeilen, arabische Verbindungen, lateinische
Ligaturen/Akzente, ZWJ-Emoji, Code, CRLF, leere und abschließende Absätze.
Geometrie, Höhe, Source-Offsets, Cache und Glyphen werden bei drei Schriftgrößen
und zwei Breiten geprüft. Tatsächliche Widgetbilder werden links/mittig/rechts
pixelgenau verglichen; absichtlich zu kurze Flächen dürfen darunter nichts
zeichnen. Reader- und einfache Cachepolitik werden mit denselben Schlüsseldaten
abwechselnd abgefragt. Ein Rohpuffer ohne NUL-Abschluss ist Teil der Prüfung.

Weiter offen: native Zeichenrechtecke, Unicode-Wortnavigation, reale
Eingabemethoden und Screenreader, weitere Geräte-/Skalierungsfälle sowie große
reale Dateien und Langzeitmessungen. Der vollständige Release-Auftrag bleibt offen.
