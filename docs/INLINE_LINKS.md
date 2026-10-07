# Gemeinsame Linkerkennung

Stand: 7. Oktober 2026, Entwicklungsschritt 0.9.5.

Leseansicht, Titel und Sternkarte benutzen denselben eigenen C-Leser für
Inline-Code, maskierte Satzzeichen, Inline-Links und Bildverweise. Die Sternkarte
verwendet zusätzlich dieselben [Blockregeln](MARKDOWN.md), damit eingerückter
und fenced Code keine Beziehungen erzeugen. Editor und Dateien bleiben unverändert.

## Regeln und Darstellung

Code-Spannen brauchen gleich lange Backtick-Abschlüsse. Maskierungen außerhalb
von Code werden berücksichtigt; in Code bleiben sie Text. Ein unbeendeter
Backtick versteckt keine späteren gültigen Links. Im aufbereiteten Codetext
werden Zeilenenden normalisiert und gegebenenfalls je ein Rand-Leerzeichen entfernt.

Linkbeschriftung, Ziel und optionaler Titel werden getrennt erkannt.
Klammern und maskierte Zeichen im Ziel sowie Ziele in Winkelklammern werden
berücksichtigt. Verschachtelte Links aktivieren den inneren Link; Bilder erzeugen
keine Linkaktion oder Sternkartenbeziehung. Ihre Alternativtexte werden angezeigt,
die Bilder selbst noch nicht gerendert. Referenzlinks, Entities,
E-Mail-Autolinks und vollständige Container-Regeln bleiben offen. Winkel-URLs
werden ab 0.9.11 als Autolinks erkannt. Ab 0.9.11
werden [Hervorhebungen und Inline-Code](INLINE_STILE.md) als eigene Stilbereiche
verarbeitet und dargestellt; rohe HTML-Tags bleiben Literaltext. Die neue
Plattform-/Paketabnahme folgt separat.

Die Regeln wurden vor der Änderung an
[CommonMark 0.31.2](https://spec.commonmark.org/0.31.2/#links) und
[Code spans](https://spec.commonmark.org/0.31.2/#code-spans) geprüft.
Der Grenzfall maskierter erster Backticks wurde zusätzlich am Verhalten der
[Referenzimplementierung](https://github.com/commonmark/commonmark.js/blob/0.31.2/lib/inlines.js)
nachgelesen. Implementierung und Prüffälle sind eigener C-Code.

Ein Link in der ersten Überschrift bleibt in der Leseansicht erreichbar,
auch wenn deren Text im festen Kartenkopf steht. Eine Überschrift wird nur
dort zusammengeführt, wenn sie vollständig mit der sichtbaren Beschriftung
übereinstimmt. Im KI-Kontext bleibt die erste Projektüberschrift im Inhalt
sichtbar. Lange oder andere Überschriften werden im Dokument vollständig gezeigt.

## Grenzen und Fehler

Pro Textblock sind jeweils 65.536 Backtick-Folgen und Klammeröffnungen zulässig.
Zielklammern und rekursive Alternativ-/Beschriftungstexte haben jeweils eine
Tiefe von höchstens 32. Begrenzte Sucharbeit verhindert unverhältnismäßigen
Aufwand bei fehlerhaften Zielstrukturen. Bei überschrittener Komplexität wird
der Fehler gemeldet; die Leseansicht erhält den Quelltext und erzeugt keine
teilweise erkannten Linkaktionen. Die Sternkarte behält ihren vorherigen
gültigen Aufbau. Die bestehenden Pfad- und Dateigrenzen gelten zusätzlich.
[Vertrag](DATENVERTRAG.md). Dies ist keine vollständige CommonMark-/GFM-Abnahme.

## Prüfausführung

Kernfälle prüfen normale, maskierte und verschachtelte Links, Inline-Code,
Bilder, Titel, Ziele mit Leerzeichen/Klammern und Originalbytes. 5.000
zusätzliche deterministische Eingaben prüfen Bereiche und Fortschritt.
Native UI-Prüfungen zählen die tatsächlichen Aktionen und öffnen eine lokale
Quelle mit optionalem Linktitel. Einzelheiten stehen im [Umsetzungsstand](STATUS.md).

Der Tastaturtest berechnet weiterhin beide Tastenphasen einschließlich Layout,
Animation und nativem Snapshot. Er zeichnet den abgeschlossenen Zustand nach
Key-up; Zwischenbefehle von Key-down werden verworfen. Direkte Animationsframes
und alle aufgenommenen Prüfzustände bleiben vollständig gerendert. Zeitmessungen
trennen Layout und Rasterung für CI-Diagnostik. Die normale App-Ausgabe wird
nicht verändert. Dies ist kein Leistungsnachweis auf realen Nutzergeräten.
