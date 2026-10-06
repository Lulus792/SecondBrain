# UI Recherche für SecondBrain

Stand: 6. Oktober 2026. Vor dem ersten Oberflächenentwurf wurden die folgenden
Originalkapitel der Apple Human Interface Guidelines eingesehen. Die Recherche
bildet die Grundlage für den anschließenden Entwurf der C-Anwendung.

## Hierarchie und Anpassung

Apple empfiehlt, Informationen nach ihrer Bedeutung anzuordnen, verwandte
Elemente erkennbar zu gruppieren und zusätzliche Optionen schrittweise sichtbar
zu machen. Das Layout soll beim Ändern der Fenster- und Textgröße verständlich
bleiben. [Layout](https://developer.apple.com/design/human-interface-guidelines/layout)

Ableitung für SecondBrain: Die aktuelle Wissensnotiz bildet den Schwerpunkt.
Projektwechsel, Wissensbereiche und Dokumentauswahl erhalten eine erkennbare
Hierarchie. Kleine Fenster müssen weiterhin Zugang zu denselben Funktionen bieten.

## Lesen und Schreiben

Apple beschreibt Typografie als Mittel für Lesbarkeit und Informationshierarchie.
Wenige Schriftfamilien und angemessene Größen helfen, Inhalte zu unterscheiden.
Lange Texte benötigen passende Eingabekomponenten; beschriftete Felder behalten
ihren Kontext, wenn Platzhalter verschwinden.
[Typografie](https://developer.apple.com/design/human-interface-guidelines/typography),
[Textfelder](https://developer.apple.com/design/human-interface-guidelines/text-fields)

Ableitung: Lesemodus und Bearbeitung unterscheiden sich klar. Überschriften,
Absätze und Listen erhalten eine eigene Darstellung. Eingaben für Projektname
und Dokumenttitel haben bleibende Beschriftungen. Änderungen, Speicherzustand
und Fehler müssen ausdrücklich verständlich sein.

## Navigation und Aktionen

Apple empfiehlt kurze, beschreibende Navigationsbezeichnungen und eine begrenzte
Hierarchietiefe in Seitenleisten. Werkzeugleisten sollen gezielt ausgewählte
Aktionen enthalten und den aktuellen Inhalt betreffen.
[Seitenleisten](https://developer.apple.com/design/human-interface-guidelines/sidebars),
[Werkzeugleisten](https://developer.apple.com/design/human-interface-guidelines/toolbars)

Ableitung: Häufige Aktionen sind erreichbar, zusätzliche Details werden bei Bedarf
geöffnet. Projekt, Wissensbereich und ausgewähltes Dokument bleiben erkennbar.
Ungespeicherte Inhalte müssen bei einem Wechsel oder beim Beenden geschützt werden.

## Suche und Rückmeldungen

Apple empfiehlt, den Suchumfang verständlich zu machen, Ergebnisse möglichst
während der Eingabe zu aktualisieren und übersichtlich anzuordnen.
[Suchfelder](https://developer.apple.com/design/human-interface-guidelines/search-fields)

Ableitung: SecondBrain durchsucht Titel und Inhalte des aktuellen Projekts.
Ergebnisse zeigen den Dokumentbezug und führen unmittelbar zum Treffer.
Ein leerer Suchzustand erläutert, was durchsucht wird.

## Farben und Zugänglichkeit

Apple beschreibt Farbe als gezielte Hervorhebung und verlangt, Informationen
zusätzlich zur Farbe verständlich zu vermitteln. Lesbarer Kontrast, größere
Schrift und passende Beschreibungen für assistive Bedienung gehören zur
Zugänglichkeit.
[Farben](https://developer.apple.com/design/human-interface-guidelines/color),
[Barrierefreiheit](https://developer.apple.com/design/human-interface-guidelines/accessibility)

Ableitung: Eine ruhige Palette mit einem Akzent, ausdrücklich beschriftete
Speicherzustände, vergrößerbare Inhalte sowie dokumentierte Tastaturwege.
Die tatsächliche Unterstützung von Screenreadern muss separat nachgewiesen
werden; visuelle Lesbarkeit allein belegt sie nicht.

## Übertragung auf die Zielplattformen

Die Apple-Gestaltungsrichtung wird über Informationshierarchie, Abstände,
Typografie, Auswahlzustände und zurückhaltende Aktionen umgesetzt. Fensterknöpfe
bleiben beim jeweiligen Betriebssystem. Tastenkombinationen berücksichtigen
Command unter macOS und Control unter Windows und Linux.

Die konkrete Oberfläche und ihre Abnahme folgen dieser Recherche. Quellen wurden
über Apples öffentliche Dokumentationsdaten eingesehen; sie sind unter den
verlinkten Originalseiten erreichbar. Es werden keine Apple-Schriftdateien oder
SF-Symbol-Assets mit dem Projekt weiterverteilt.
