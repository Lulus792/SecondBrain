# Abschnittstrennungen in der Leseansicht

Stand: 7. Oktober 2026, laufender Schritt 0.9.10.
Vor der Gestaltung erneut gelesen: [Apple HIG Layout](https://developer.apple.com/design/human-interface-guidelines/layout),
über die öffentlichen Dokumentationsdaten, und [CommonMark 0.31.2, Thematic breaks](https://spec.commonmark.org/0.31.2/#thematic-breaks).
Apple beschreibt räumliche Gruppierung und Trennlinien als mögliche Mittel zur
Informationsgliederung. Eigene Übertragung: eine zurückhaltende waagerechte
Linie in der tatsächlichen Inhaltsbreite, mit Abstand zu benachbartem Text.
Die konkrete Linienfarbe und Höhe sind eigene Entscheidungen.

## Eigene C-Erkennung und Darstellung

Mindestens drei gleiche Stern-, Bindestrich- oder Unterstrichzeichen mit
zulässigen Zwischenräumen bilden eine Abschnittstrennung. Bis zu drei führende
Leerzeichen sind möglich; vier Spalten Einrückung bleiben Code. Eine passende
Bindestrichzeile direkt nach Absatztext bleibt eine Setext-Überschrift. Gemischte
Marker, Maskierungen und zu kurze Folgen bleiben Text. Code und der unverändernde
Rohtextmodus verwenden diese Darstellung nicht.

Der vorhandene eigene Blockleser liefert dafür einen eigenen Blocktyp. Die UI
zeichnet die Linie in der Mitte einer skalierenden Abstandzeile, innerhalb des
Reader-Clips. Kontrastdarstellung verwendet die volle vorhandene Textfarbe.
Quelltext, Editor, Speicherung und Sternkartenverweise bleiben erhalten.

## Native Struktur und Bedienung

AccessKit Splitter wird von den festgelegten Providern als AXSplitter,
UIA Separator beziehungsweise AT-SPI Separator abgebildet (Originalquellen:
AccessKit macOS node.rs, Windows 0.35.1 und AT-SPI Common 0.21.0 node.rs).
Die App veröffentlicht eine horizontale, beschriftete Abschnittstrennung ohne
Textwert und ohne Eingabe- oder Klickaktion. Sie erzeugt keinen zusätzlichen
Tabstopp; Überschriftensprünge bleiben auf Überschriften beschränkt. Die native
Geometrie entspricht der tatsächlich gezeichneten und geclippten Linie.

## Abnahme und Grenzen

Eigene Kernfälle prüfen die Abgrenzung zu Setext, Code, gemischten/kurzen/
maskierten Folgen und CRLF. Der native Prüffall verlangt genau eine Trennung,
passende Rolle/Geometrie, erhaltene Code- und Überschriftsemantik und unveränderte
Originalbytes bei 100 und 200 Prozent. Ausgeführte Ergebnisse werden nach
Abschluss in STATUS.md dokumentiert. Das schließt geschachtelte Container,
vollständige Inline-Regeln, Bidi oder menschliche Screenreader-Abnahme nicht ab.

Drei gezielte lokale Prüfungen bestehen (15,31 s): 334 native, 111 Editor- und
181.838 Blockassertions einschließlich 10.000 deterministischer Eingaben.
Die vorherige Blockerkennung scheitert am neuen Blocktyp. Eigene Blockerkennung
und Testcode bestehen unter ASan/UBSan; übriger Kern ist dabei nicht vollständig
instrumentiert. Normale und 200-Prozent-Raster wurden betrachtet. Die anschließende
Gesamtprüfung beinhaltet zusätzlich den korrigierten Hilfe-Icon-Punkt.

Die helle Kontrastnachprüfung besteht mit 340 nativen Assertions (10,67 s),
einschließlich eines tatsächlich ausgelesenen Linienpixels in der Textfarbe.
Das Raster zeigte dabei Reader-Inhalt am nach innen versetzten Fokusring.
Die Reader-Innenabstände skalieren jetzt mit der Schriftgröße; zusätzliche
Geometrieassertions verlangen Abstand auf beiden Seiten. Neuer Gesamtlauf folgt.

Nach der Paddingkorrektur besteht die gesamte lokale Release-Nachprüfung
mit allen 33 Tests (195,44 s), einschließlich realer Linienpixel und Abständen
zur inneren Fokusmarkierung. Das abschließende Kontrastraster wurde betrachtet.
Paket und native Plattformnachweise folgen separat.

Die abschließende native Teilprüfung umfasst 342 Assertions.
