# Zeichenreferenzen in Markdown

Stand: 7. Oktober 2026, Entwicklungsschritt 0.9.14.
Die vor der Änderung gelesenen Originale stehen in [Recherche](ENTITIES_RECHERCHE.md).
Benannte HTML5-Zeichenreferenzen mit Semikolon und numerische Unicode-Angaben
werden vom eigenen C-Leser dekodiert. Diese Regeln gelten für Leseabsätze,
Überschriften, Tabellen, alternative Bildtexte, Linkbeschriftungen und erkannte
Inline-Linkziele. Text-/Stilprojektion, Titel und Sternkarten verwenden denselben Leser.

## Vertrag

Alle 2.125 festgelegten Semikolon-Namen stammen aus unveränderten WHATWG-Daten.
Dezimale Referenzen erlauben 1–7, hexadezimale 1–6 Ziffern; Null, Surrogate und
Werte oberhalb U+10FFFF werden U+FFFD. Andere Unicode-Skalarwerte bleiben die
angegebenen Zeichen, einschließlich C1-Kontrollen; daraus wird kein vollständiger
HTML-Tokenizer oder dessen historische Windows-1252-Umsetzung abgeleitet.
Unbekannte Namen, fehlende Semikolons und ungültige Ziffernfolgen bleiben Literaltext.

Code und maskierte Referenzen werden nicht dekodiert. Rohe HTML-Tags bleiben
unverändert als Literaltext sichtbar. Autolinks behalten ihre literalen Referenzen,
auch wenn nur die Beschriftung eines solchen Tokens gelesen wird. Dekodierte
Sterne, Klammern und Hashzeichen werden nicht erneut als Markdown-Struktur gelesen.
Linkziele werden erst nach ihrer syntaktischen Erkennung dekodiert; Maskierungen
und die vorhandenen Pfad-/Dateiregeln bleiben erhalten. Nicht dargestellte Code-
Sprachhinweise und noch fehlende Referenzlinks sind separate Parserarbeiten.

Die Klartextprojektion verwendet den bisherigen Softline-Vertrag: CR/LF werden
zu Leerzeichen, auch aus Referenzen. Diese Anzeige ist keine Änderung der
Originaldatei. Zielpfade behalten dekodierte Bytes und durchlaufen ihre normale
Pfadbehandlung. Stiloffsets beziehen sich auf die ausgegebenen UTF-8-Bytes.

## Speicher und Darstellung

&nGt; und &nLt; erzeugen je sechs Ausgabebytes aus fünf Quellbytes. Geprüftes
Wachstum ersetzt deshalb die Annahme einer stets kürzeren Ausgabe. Die 16-MiB-
Textgrenze gilt auch für aufbereiteten Text. Überschreitung gibt einen benannten
Fehler und keine halbe Projektion zurück. Editor und Quelle bleiben erhalten.
Linkzielpuffer prüfen ihre Grenze einschließlich abschließendem NUL-Byte.

Noto Sans Math ergänzt die vorhandenen UI-Ersatzschriften: Google-Fonts-Commit
823468bd7825152bce2b8fd2cf740432ad2fce8d, Originalfont-SHA-256
`3f495fe933c06786e4d5f6d86b8ee70b6753a68ee3b9d87528726de0f6e2c47d`.
Die SIL-OFL-Datei hat SHA-256
`403a95275b469061b7d4371c328e0ada3bc7d63328abe2e88aad5cd243b2fe21`.
Vollständige Grapheme verwenden weiterhin eine abdeckende Schrift; die beiden
expandierenden Mathematikzeichen erschienen vorher als Kästchen und werden
mit der neuen Schrift tatsächlich gerastert. Dies ist keine Zusage für alle
Unicode-Glyphen. Bidi, vollständige Fallback-/Zeichengeometrie und IME bleiben offen.

## Nachweise und Grenzen

Der unabhängige Datenvergleich liest alle 2.125 Namen über die tatsächliche
C-Probe und prüft auch alle Namen innerhalb von Code. 15 ausgewählte originale
CommonMark-Zeichenreferenzfälle bestehen; Referenzlink- und Listenfälle bleiben
bis zu deren Umsetzung offen. Rohes HTML folgt dem Literalvertrag. Ein allein
vom HTML-Renderer angehängter Code-Endzeilenumbruch wird beim Vergleich nicht
als zusätzlich dargestelltes Zeichen verlangt. Die originalen Fälle bleiben
unverändert. [Datenherkunft/Lizenz](../tests/data/commonmark-0.31.2/README.md).

Eigene Grenzfälle prüfen maskierte, ungültige, numerische und expandierende
Referenzen, passende Stile, erhaltene Quellen, Zielpuffer und 16-MiB-Grenze.
Echte App-Prüfungen lesen dekodierten nativen Text, Tabellen und Titel und öffnen
ein kodiertes lokales Ziel mit Umlauten per Tastatur. Sternkartenprüfung erhält
genau die wirkliche Beziehung und keine aus kodierten Strukturmarkern.
Schriftmetriken und betrachtete Raster prüfen die Mathematikkombinationen.
Ausgeführte endgültige Ergebnisse stehen in [STATUS](STATUS.md).
