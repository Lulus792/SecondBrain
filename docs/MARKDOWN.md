# Markdown in der Leseansicht

Stand: 7. Oktober 2026, Entwicklungsschritt 0.9.4. Die Anwendung liest
Markdown selbst in C. Sie verändert die Quelldatei beim Anzeigen nicht.
Editor und Sicherung behalten die Originalbytes; Anzeigen ist keine Migration.

## Blockregeln

Der neue UI-unabhängige Blockleser liefert Quellpositionen und Textbereiche
ohne Kopien oder Schreibzugriffe. Die Leseansicht, ihre nativen Textblöcke und
die Titelermittlung benutzen dieselbe Erkennung. Zeilenenden LF, CRLF und CR
werden erkannt; weiche Umbrüche im aufbereiteten Absatz werden zu Leerzeichen.

Ein bis sechs führende Hashzeichen kennzeichnen ATX-Überschriften. Optional
abschließende Hashzeichen werden entfernt, wenn sie vom Text getrennt sind.
Setext-Unterstreichungen erkennen auch mehrzeilige Überschriften.
Codezäune erlauben Backticks oder Tilden und merken sich Zeichen, Länge und
Einrückung. Nur ein passender Abschluss beendet den Block; ein kürzerer Zaun
oder Text nach dem Abschluss bleibt Code. Ein unbeendeter Zaun reicht bis zum
Dokumentende. Eingerückter Code wird als solcher angezeigt; innerhalb eines
bestehenden Absatzes erzeugt bloße Einrückung keinen neuen Codeblock.

Diese Regeln wurden vor der Änderung am Original
[CommonMark 0.31.2](https://spec.commonmark.org/0.31.2/) überprüft, insbesondere
ATX-/Setext-Überschriften, Codeblöcke und Absätze. Tests sind eigene Fälle,
keine übernommene vollständige Konformitätssuite.

Zeilen mit tatsächlichen Listenmarkern, Zitaten, Tabellen oder Trennlinien
werden noch getrennt dargestellt. Normale Zahlen am Absatzanfang lösen keine
Trennung mehr aus. Die vollständigen Container-Regeln, geschachtelte Listen,
Tabellenstruktur, Inline-Regeln, Entities, Referenzlinks und HTML-Blöcke sind
noch nicht vollständig umgesetzt. Die Anwendung führt kein HTML aus.
Dies ist weiterhin eine begrenzte Markdown-Leseansicht, keine vollständige
CommonMark- oder GFM-Implementierung. Der Auftrag für die vollständige
Release-Abnahme bleibt damit offen.

## Darstellung und Navigation

Die vorhandenen Schriftrollen, Lumen-Karten, Linkschaltflächen und Scrollwege
bleiben die Grundlage. Codeinhalt erzeugt weder Markdown-Überschriften noch
Linkaktionen. Überschriften werden mit ihrer tatsächlichen Ebene veröffentlicht;
Alt+Bild auf/ab und native Abschnittsanfragen verwenden weiterhin Quellpositionen.
Nicht-Markdown-Quellen und Lizenztexte benutzen den unverändernden Textmodus.
[Native Struktur](DOKUMENTSTRUKTUR.md), [Textgrenzen](TEXTDARSTELLUNG.md).

## Prüfung

Die Kernprüfung umfasst klare Öffnungs-/Abschlussregeln, Überschriften,
Zeilenenden, fortgesetzte Absätze, Titelpuffergrenzen und 10.000 deterministische
Eingaben mit gültigen Quellbereichen und unverändertem Eingabetext. Die eigene
C-Implementierung wird zusätzlich unter ASan/UBSan geprüft. Die native UI-Prüfung
vergleicht echte Rollen, Text und gespeicherte Originalbytes, einschließlich
Code mit scheinbarer Überschrift und scheinbarem Link. Konkrete ausgeführte
Ergebnisse und Plattformgrenzen stehen im [Umsetzungsstand](STATUS.md).
