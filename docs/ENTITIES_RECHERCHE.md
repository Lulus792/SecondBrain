# Zeichenreferenzen für den nächsten Markdown-Schritt

Stand: 7. Oktober 2026. Historische Recherche vor dem Implementierungsschritt.
Die Umsetzung und aktuelle Nachweise stehen in [ENTITIES.md](ENTITIES.md).

## Geprüfte Grundlagen

[CommonMark 0.31.2, Zeichenreferenzen](https://spec.commonmark.org/0.31.2/#entity-and-numeric-character-references)
verlangt benannte HTML5-Referenzen mit Semikolon sowie dezimale Referenzen mit
1–7 und hexadezimale mit 1–6 Ziffern. Ungültige Skalarwerte und Null werden durch
U+FFFD ersetzt. Code bleibt Literaltext; dekodierte Zeichen werden nicht zu
Markdown-Strukturmarkern. Linkziele werden erst nach ihrer syntaktischen
Erkennung dekodiert. Rohe HTML-Attribute behalten ihre Referenzdarstellung.
Referenzlinks und deren Dokumentkontext sind eine zusätzliche Parseraufgabe.

Die [WHATWG-Daten](https://html.spec.whatwg.org/entities.json) sind am 7. Oktober
als Original gelesen: SHA-256 `d741d877ac77c4194c4ad526b5b4a19aef8dfe411ab840a466891cdbb9f362e6`.
Sie enthalten 2.125 Semikolon-Namen; längster vollständiger Name 33 Bytes.
Zwei Referenzen, &nGt; und &nLt;, erzeugen sechs UTF-8-Bytes aus fünf Quellbytes.
Die bisherige Annahme „Ausgabe passt stets in die Quelllänge“ reicht damit nicht.

Die [Original-Lizenz](https://github.com/whatwg/html/blob/main/LICENSE)
(SHA-256 `85dc6f5ccb57a6fe8c33d158f9fc8fc7ee5655a5d3db2cdd131c6a3d0f48a864`)
lizenziert Dokumentmaterial unter CC BY 4.0 und in Quellcode übernommene Teile
unter BSD 3-Clause. Original und abgeleitete Datentabelle benötigen die
entsprechende Zuordnung; die eigene Parserimplementierung bleibt MIT. Daten
sind keine neue fachliche Bibliothek. Quellen liegen vorerst im ignorierten
build/entities-research; noch nicht Bestandteil der App.

## Nächste Umsetzung und Abnahme

Eigener C-Decoder, erzeugte festgelegte Tabelle und bounded lookup. Ausgabe
mit geprüftem Wachstum bis zur tatsächlichen SB_TEXT_LIMIT-Grenze (16 MiB),
Stiloffsets in ausgegebenen UTF-8-Bytes und atomischer Freigabe bei Fehlern.
Quelltext und ursprüngliche Block-/Linkerkennung erhalten. Code, Maskierungen,
rohe HTML-Elemente und Autolinks mit ihrem jeweiligen Vertrag getrennt prüfen.
Keine Wiederauswertung dekodierter Strukturzeichen.

Alle offiziellen Semikolon-Namen unabhängig gegen die Quelle prüfen; numeric,
ungültige, maskierte und expandierende Fälle einschließlich Grenzspeicher.
Titel, Tabelle, native Texte, lokale Linkziele und Sternkartenbeziehungen mit
UTF-8-Dateinamen abnehmen. Normfälle und ursprüngliche Quelldateien erhalten.
Neue Datenlizenz in Paket und App-Lizenzansicht aufnehmen. Die Anforderung ist
noch offen; aus dieser Recherche wird keine Implementierungsabnahme abgeleitet.
