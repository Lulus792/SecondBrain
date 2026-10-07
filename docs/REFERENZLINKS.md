# Referenzlinks im gemeinsamen C-Leser

Stand: 7. Oktober 2026, Entwicklungsschritt 0.9.16. Die eigene Anwendung verwendet
Referenzdefinitionen aus der ganzen Datei für Titel, Leseansicht, Tabellen,
native Texte und Sternkartenbeziehungen. Quelldateien bleiben unverändert.

```markdown
[Quelle][Entscheidung]
[Entscheidung][]
[Entscheidung]

[Entscheidung]: knowledge/entscheidung.md "Hintergrund"
```

Die erste passende Definition gewinnt, auch wenn spätere Definitionen denselben
Namen mit anderer Großschreibung verwenden. Eine gültige Definition erscheint
nicht als zusätzlicher Absatz. Namen werden mit vollständiger Standard-
Unicode-Faltung aus festgelegten Unicode-18.0.0-Daten verglichen; zum Beispiel
passen „Straße“, „STRASSE“ und „Straẞe“ zusammen. Rand-Leerzeichen, Tabulatoren und
Zeilenenden entfallen, innerer ASCII-Leerraum wird zusammengefasst. Unicode-Namen
haben höchstens 999 Zeichen. Unmaskierte innere eckige Klammern sind unzulässig.
Maskierungen und Entities bleiben im Vergleich Bestandteil der Namensnotation;
sie werden nicht wie der sichtbare Lesetext dekodiert. Keine zusätzliche NFC-
Normalisierung und keine sprachabhängige türkische Sonderfaltung.

## Definitionen und Linkvorrang

Die gemeinsame Blockerkennung findet Definitionen am Absatzanfang mit höchstens
drei Spalten Einrückung. Eine Definition unterbricht keinen laufenden Absatz.
Codeblöcke und Codezäune erzeugen keine Definitionen. Namen, Ziele und optionale
Titel dürfen nach ihren Grammatikregeln über Folgezeilen gehen; leere Zeilen
brechen die Erkennung. Klammern, Winkelziele und maskierte Satzzeichen folgen
den Zielregeln. Ein ungültiger Titel auf derselben Zeile verwirft die Definition;
ein ungültiger Folgetitel kann als sichtbarer Text erhalten bleiben.

Vollständige, verkürzte und einfache Referenzlinks sowie die entsprechenden
Bildnotationen verwenden dieselbe unverändernde Umgebung. Inline-Links haben
Vorrang. Bilder zeigen weiterhin aufbereiteten Alternativtext; sie erzeugen
keine eigene Linkaktion oder Graphkante. Innere Links verhindern einen äußeren
zusätzlichen Link. Unbekannte Referenzen bleiben sichtbare Notation. Leere Ziele
bleiben im Parser erkannt, erzeugen wie bisher keine lokale Quellenaktion.
Linkziele dekodieren Maskierungen und Entities vor der bestehenden Pfadprüfung.

Raw-HTML bleibt im eigenen Reader Literaltext. Der Parser führt es nicht aus.
Bei weichen Zeilenumbrüchen werden angrenzende rohe Leerzeichen/Tabulatoren
entfernt, bevor der Umbruch als Leerzeichen erscheint. Code, rohe HTML-Tags,
Autolinks und dekodierte Zeichenreferenzen behalten ihre jeweiligen Verträge.
Harte Markdown-Zeilenumbrüche und vollständige HTML-Blockregeln bleiben offen.

## Grenzen und Bestandsschutz

Pro Datei sind höchstens 65.536 erkannte Definitionszeilen und 16 MiB gespeicherte
normalisierte Namen zulässig. Auch doppelte Definitionen zählen vor ihrer
Zusammenführung gegen diese Grenzen. Die Definitionserkennung und Linkarbeit
haben eine von der Quellenlänge abhängige Arbeitsgrenze. Ziele behalten maximal
32 Ebenen verschachtelter Klammern. Bei Fehlern bleibt die Originaldatei erhalten;
die Leseansicht zeigt den Quelltext im Literalmodus, der Graph behält seinen
letzten gültigen Stand. [Bestehende Daten-/Dateigrenzen](DATENVERTRAG.md).

Ab 0.9.17 sammelt der [Dokumentbaum](DOKUMENTBAUM.md) auch Definitionen
innerhalb von Zitaten und Listen. Titel, Reader, Tabellen und Graph verwenden
die zugehörigen Blattprojektionen.
Dieser Schritt ist deshalb keine vollständige CommonMark-/GFM-Abnahme.

## Quellen und Nachweise

Vor Umsetzung gelesen: [CommonMark 0.31.2, Definitionsblöcke](https://spec.commonmark.org/0.31.2/#link-reference-definitions)
und [Referenzlinks](https://spec.commonmark.org/0.31.2/#links), 7. Oktober 2026.
Die ursprüngliche [Recherche](REFERENZLINKS_RECHERCHE.md) bleibt als Ausgangspunkt erhalten.

Originaldaten: [Unicode 18.0.0 CaseFolding](https://www.unicode.org/Public/18.0.0/ucd/CaseFolding.txt),
SHA-256 `a004797658a457bec4dc11683e39f69249ea3b595b752dbea6721c4c9f587b0d`.
Der eigene Entwicklergenerator erzeugt die C-Tabelle aus den C-/F-Mappings nach
Hashprüfung; Runtime und normaler C-Build benötigen kein Python und keine externe
Unicode-Bibliothek. Die unveränderten Daten und die abgeleitete Tabelle fallen
unter die bereits mitgelieferte Unicode License V3. [Zuordnung](../third_party/unicode/README.md).

81 unveränderte Normfälle vergleichen tatsächliche C-Texte, Stile und Ziele.
Zwei Raw-HTML-Fälle verwenden den Literalvertrag, zwei Bildfälle den bestehenden
Stilvertrag des Alternativtexts. Terminale HTML-Code-Serializer-Zeilenumbrüche
werden wie im bisherigen Vertrag behandelt; Originalfälle werden nicht verändert.
Die Container-Definitionsfälle sind ab 0.9.17 Teil des zusätzlichen 307er-
Baumvergleichs; dieser prüft Struktur und Klartext, keine vollständigen Inline-
Stile/Ziele. Eigene Integrationstests prüfen tatsächliche Containerziele. Alle 1.606 Original-
Casefold-Mappings bestehen gegen tatsächliche C-Ausgabe. Eigene Fälle prüfen
Unicode-Expansion, Puffer, Quellen, Definitionenzahl und 3.000 begrenzte Eingaben.
Native Reader-/Graphprüfungen umfassen Titel, Absatz, Zelle, erste Definition,
Code, echte lokale Umlaut-Ziele und Tastatur-Rückkehr. Ausgeführte Tests und
Plattformgrenzen stehen im [Umsetzungsstand](STATUS.md).
