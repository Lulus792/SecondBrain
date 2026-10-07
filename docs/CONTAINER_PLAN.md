# Dokumentbaum für Listen und Zitate

Stand: 7. Oktober 2026. Fortsetzung des vollständigen Markdown-Auftrags,
keine fertige UI-Funktion und keine abgeschlossene Container-Abnahme.

## Bestätigter Ausgangspunkt bis 0.9.16

Der bisherige Blockleser liefert einen direkten Bereich der Quelldatei pro
Block. Listenmarker und Zitatpräfixe bleiben dabei häufig im Absatztext. Die
Leseansicht hat dafür noch keine vollständige Baumstruktur und native
Listensemantik. Der vorhandene Referenzleser findet Definitionen in den flachen
Blöcken; Definitionen innerhalb vollständiger Container bleiben offen.

Ein Entfernen aller führenden `>`/Listenmarker wäre unzureichend: Fortsetzungen,
Code, Tabellen, echte Leerzeilen und Definitionen haben je nach Container einen
anderen Kontext. Außerdem gingen Quellpositionen verloren, die Überschrift-
Navigation und native Kennungen heute verwenden. Der Originaltext gehört weiter
in Editor, Datei und Sicherung. Eine aufbereitete Parseransicht ist eine getrennte,
kurzlebige Datenstruktur.

## Vor der Umsetzung gelesene Originale

[CommonMark 0.31.2, Block quotes](https://spec.commonmark.org/0.31.2/#block-quotes),
[List items](https://spec.commonmark.org/0.31.2/#list-items),
[Lists](https://spec.commonmark.org/0.31.2/#lists) und
[Parsing strategy](https://spec.commonmark.org/0.31.2/#appendix-a-parsing-strategy),
7. Oktober 2026. Ergänzend die offizielle
[Referenzimplementierung commonmark.js 0.31.2, blocks.js](https://github.com/commonmark/commonmark.js/blob/0.31.2/lib/blocks.js),
insbesondere physische Tabspalten, teilweise verbrauchte Tabs, Absatzfortsetzungen,
Listengruppierung und Referenzdefinitionen vor Setext-Umwandlung. Sie wird studiert,
nicht als Laufzeitbibliothek eingebunden oder in den eigenen Kern kopiert.

Der unveränderte Originaldatensatz enthält 25 Blockquote-, 48 List-item- und
26 List-Fälle. Seine bereits festgelegte Herkunft/Lizenz steht unter
[tests/data/commonmark-0.31.2](../tests/data/commonmark-0.31.2/README.md).
Diese 99 Fälle sind inzwischen Teil des 307er-Baumvergleichs. Die
[Kernimplementierung und weitere offene Integration](DOKUMENTBAUM.md) sind dort
getrennt dokumentiert; daraus folgt noch keine App-Abnahme.

## Implementierte Voraussetzung: Quellprojektion

`src/projection.c` und `projection.h` liefern eigenen C-Code für eine geliehene
Originalquelle und eine davon getrennte, besessene Textansicht. Rohbereiche
bleiben bytegenau. Teilweise entfernte Tabulatoren erhalten die verbleibenden
Leerzeichen; ihre Zeichen verweisen auf den ursprünglichen Tabulator. CR, LF und
CRLF können für den Parser als LF vorliegen und behalten ihre Quellposition.
Ein zusätzlicher terminaler Parserumbruch darf auf das Dateiende verweisen.

Der Cursor zählt physische Tabspalten mit Vierer-Tabstopps, statt beim Entfernen
eines Präfixes den Spaltenstand zurückzusetzen. Vollständige spätere Tabs bleiben
roh erhalten. Die Quellsuche erfolgt über geordnete Ansichtsbereiche. Benachbarte
bytegleiche Quellkopien werden zusammengeführt. Fehlgeschlagene Anfügungen
verändern weder sichtbaren Text noch die aktive Positionszuordnung; Kapazität
kann sich bei einer fehlgeschlagenen Speicherreserve ändern. Originalbytes werden
niemals beschrieben. Die Originalquelle muss während der Ansicht erhalten bleiben.

Ein einzelner Projektionspuffer hat eine geprüfte Daten-/Bereichsgrenze:
höchstens 16 MiB plus einen terminalen Parserumbruch und 65.536 getrennte
Zuordnungsbereiche. Dies ist die aktuelle interne Bausteingrenze, noch keine
Abnahme oder endgültige Zusage für vollständige Containerdateien. Der spätere
Dokumentleser muss Gesamtarbeit, Baumgröße und benötigte Zuordnungen gemeinsam
prüfen; bei Fehlern sind Quelle und letzter gültiger Graph zu erhalten.

Ab 0.9.17 ist die API über den Dokumentbaum mit der Leseansicht verbunden.
[Listen-/Zitatdarstellung](CONTAINER_UI.md) und [STATUS](STATUS.md) nennen
den neuen Umfang und Nachweise. Die installierte App bleibt bis zur neuen
Paketabnahme bei 0.9.16.

## Folgende Umsetzungsstufen

1. Eigenen Dokumentbaum mit Container-/Blattknoten und ursprünglichen Bereichen
   auf der geprüften Projektion aufbauen. Quote, Liste und Listeneintrag brauchen
   Eltern-/Geschwisterbeziehungen; Überschriften, Absätze, Code, Trennungen,
   Definitionen und Tabellen bleiben Blattaufgaben.
2. Offene Container je Zeile fortsetzen oder schließen; fehlende Präfixe nur bei
   erlaubter Absatzfortsetzung akzeptieren. Einrückung, leere Einträge,
   Listenbeginn/-wechsel, kompakte/lockere Listen und Codegrenzen normnah prüfen.
   Vorhandene Blattregeln sollen gemeinsam bleiben, nicht in zwei Parsern driften.
3. Referenzen im ganzen Baum sammeln. Definitionen bleiben an ihren Blattkontext
   und dessen besessene Projektion gebunden; ein ungegliederter Gesamttext wäre
   für Unterbrechungsregeln unzureichend. Erste Definition und Unicode-Vergleich
   bleiben dieselben Verträge.
4. Titel, Sternkarte, Tabellen und Testprobe auf diese Umgebung umstellen.
   Ursprüngliche Positionen bleiben Grundlage von Kennungen und Abschnittssprüngen.
5. Nach entsprechender UI-Recherche Listen/Zitate sichtbar gliedern und nativ als
   Container mit Kindern veröffentlichen. Tastaturaktionen und Überschriften-
   Navigation bleiben erreichbar; große Schrift und schmale Fenster prüfen.
6. Originalfälle, Grenz-/Bestandsschutz, Sanitizer, Raster, native Werte und
   tatsächliche Plattformpakete abnehmen. Erst danach bisherige Container-
   Aufgaben als abgeschlossen kennzeichnen.

Die übrigen [Release-Aufgaben](RELEASE.md) einschließlich Bidi/Geometrie/IME,
Tabellenmatrix und menschlicher assistiver Bedienung bleiben ebenfalls bestehen.
