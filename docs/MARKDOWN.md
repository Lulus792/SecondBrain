# Markdown in der Leseansicht

Stand: 7. Oktober 2026, Entwicklungsschritte 0.9.4–0.9.17. Die Anwendung liest
Markdown selbst in C. Sie verändert die Quelldatei beim Anzeigen nicht.
Editor und Sicherung behalten die Originalbytes; Anzeigen ist keine Migration.

## Blockregeln

Der UI-unabhängige [Dokumentbaum](DOKUMENTBAUM.md) liefert Quellpositionen und
getrennte Textprojektionen ohne Schreibzugriffe auf das Original. Die Leseansicht, ihre nativen Textblöcke und
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

Ab 0.9.6 werden [Tabellen](TABELLEN.md) als eigener Block erkannt und gelesen.
[Abschnittstrennungen](TRENNLINIEN.md) werden ab 0.9.10 als waagerechte Linien
mit passender nativer Semantik dargestellt. Ab 0.9.17 verarbeitet der
Dokumentbaum verschachtelte Listen, Zitate, Tabellen und Definitionen darin.
Normale Zahlen am Absatzanfang lösen keine Trennung aus. Die vollständige
Markdown-/Inline-Abnahme bleibt offen. HTML-Blöcke werden literal angezeigt;
die Anwendung führt kein HTML aus. Ab 0.9.5 verwenden Leseansicht, Titel und
Sternkarte gemeinsame [Inline-/Linkregeln](INLINE_LINKS.md); die dort benannten
weiteren Regeln bleiben offen. Ab 0.9.11 stehen gemeinsame
[Hervorhebungen und Inline-Code](INLINE_STILE.md) zur Verfügung.
0.9.14 ergänzt [Zeichenreferenzen](ENTITIES.md), 0.9.15
[E-Mail- und URI-Autolinks](AUTOLINKS.md). 0.9.16 ergänzt
[Referenzdefinitionen und ihre gemeinsame Umgebung](REFERENZLINKS.md).
Dies ist weiterhin eine begrenzte Markdown-Leseansicht, keine vollständige
CommonMark- oder GFM-Implementierung. Der Auftrag für die vollständige
Release-Abnahme bleibt damit offen. [Containerplan und Quellprojektion](CONTAINER_PLAN.md)
dokumentieren ihre schreibgeschützte Grundlage. [Container-UI](CONTAINER_UI.md)
nennt die implementierte Gestaltung und ihre Prüfgrenzen.

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

## Listen und Zitate ab 0.9.17

Ein eigener Containerbaum erkennt Listen, Einträge und Zitate einschließlich
Fortsetzungen, Einrückung und Referenzdefinitionen. Die Darstellung verwendet
Marker/Nummern und eine Zitatlinie; native Container gruppieren die Textblätter.
Code und HTML bleiben literal. Titel und Sternkartenlinks benutzen denselben
Baum. [Regeln, Gestaltung und Grenzen](CONTAINER_UI.md). Die geprüften
307 ausgewählten CommonMark-Fälle belegen keine vollständige Markdown-Abnahme.
