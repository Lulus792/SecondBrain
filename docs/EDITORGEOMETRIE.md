# Gemeinsame Geometrie in Editor und Eingabefeldern

Stand: 8. Oktober 2026, Entwicklungsschritt 0.9.35. Die eigene C-Anbindung
verwendet den [Glyphenplan](GLYPHENGEOMETRIE.md) und seine
[Cursorpunkte](CURSORGEOMETRIE.md) jetzt in den produktiven Nuklear-Eingaben.
Ergebnis und Plattformgrenzen nennt [STATUS](STATUS.md).

## Grundlage

Apple [Text views](https://developer.apple.com/design/human-interface-guidelines/text-views),
am 8. Oktober über die offiziellen Dokumentationsdaten erneut gelesen:
Längere und bearbeitbare Inhalte benötigen eine passende Textansicht; Lesbarkeit
soll auch bei geänderter Schriftgröße geprüft werden, nützlicher Text soll
kopierbar sein. Caches, Bidi-Affinität und Rasterverfahren sind eigene
plattformübergreifende Entscheidungen, keine Apple-Vorgaben.

Der bisherige Editor maß Cursor und Maus teilweise aus Einzelzeichen oder
separaten Präfixen. Ausgewählte Textstücke wurden getrennt gezeichnet. Das
verliert Kerning, Ligaturpositionen oder arabische Verbindungen und kann bei
Schriftwechseln zur gezeichneten Zeile abweichen.

## Produktive Anbindung

[app/edit_geometry.inc](../app/edit_geometry.inc) arbeitet mit einer unveränderten
UTF-8-Kopie. Sie erstellt je Absatz Richtung, ganze Grapheme, Script-/Schriftwahl,
Glyphen und Cursorpunkte. Ein eigener Index bildet die logischen Unicode-
Zeichenpositionen der Nuklear-Auswahl auf UTF-8-Bytes ab. Der Quelltext wird
nicht visuell umsortiert; Undo und gespeicherte Markdown-Dateien bleiben logisch.

Die Anbindung gilt für Editor, Suche, Notiz-/Projektfelder und die übrigen
Nuklear-Textfelder. Der Editor behält explizite Zeilenwechsel und horizontales
Scrollen; dies ist keine Einführung eines weichen Editorumbruchs.

- Links/rechts bewegen sich zwischen visuellen Cursorpunkten. Bei RTL kann
  dabei der logische Quellindex steigen. Zeilenränder folgen der Absatzrichtung;
  ein CRLF wird als vollständige Einheit überquert.
- Auf/ab behalten die visuelle x-Position als Ziel. Zeilenanfang/-ende verwenden
  die logischen Grenzen der Zeile. Wort- und Dokumentbefehle behalten ihre
  bestehenden Regeln; vollständige Unicode-Wortnavigation bleibt gesondert offen.
- Mauspositionen bleiben Float-Werte. Klick und Ziehen verwenden die Punkte der
  tatsächlich geformten Zeile. Eine Auswahl in einer Zeile wird beim Aufheben
  in der gewählten visuellen Richtung zusammengeklappt.
- An einer Richtungsgrenze werden Seite und Level gehalten. Ein Schriftgrößen-
  wechsel berechnet die Position neu und erhält die gewählte Seite.
- Auswahlflächen werden vor dem Zeichnen vereinigt. Die vollständige geformte
  Zeile erhält unterschiedliche Farbclips; ausgewählte Teilstrings werden nicht
  neu geformt. Jeder Bildpunkt wird dabei einmal gezeichnet, damit die
  Antialiasing-Kanten nicht durch doppeltes Übermalen heller werden.
- Einfügen zeichnet einen dünnen Cursor an derselben Position. Überschreiben
  markiert die nächste Graphemeinheit und färbt ihre vorhandenen Glyphen über
  einen Clip. Gefüllte Rechtecke und Scissor-Kommandos behalten Float-Geometrie.
- Der Renderer skaliert die temporären Vertices einmal in Backing-Pixel und
  setzt Clips an gemeinsamen physischen Pixelgrenzen. Danach stellt er den
  ursprünglichen SDL-Maßstab und Clipzustand wieder her. Eine fehlgeschlagene
  Kommandokonvertierung wird vor dem Lesen ihrer Vertexdaten abgebrochen.
- Zeilenhöhe berücksichtigt Ersatzschriftmetriken. Ein Tabulator bleibt im
  Quelltext ein Zeichen und belegt vier Leerzeichenbreiten; an seiner Stelle
  wird keine Ersatzglyphe gezeichnet. Variable Tabstopps sind nicht implementiert.

Die vorläufige IME-Anzeige benutzt denselben Plan mit ihrem eigenen Anzeigetext.
Markierungen und Auswahl folgen den tatsächlichen Flächen. Der Cursor folgt dem
angegebenen Start der IME-Auswahl; deren Länge verschiebt den Eingabeanker nicht.
Originaltext und Undo ändern sich erst bei der Bestätigung. Der native
SDL-Eingabeanker kommt weiterhin aus dem tatsächlich gezeichneten Cursor und
wird in Fensterpunkten angegeben. Reale Eingabemethodenabnahme bleibt erforderlich.

## Cache und Lebensdauer

Acht exakte Einträge halten Text, Font, angeforderte Zeilenhöhe und Feldtyp.
Hashes ersetzen den vollständigen Bytevergleich nicht. Das zurückgehaltene
Cachebudget beträgt 64 MiB. Größere Pläne werden als frische temporäre Pläne
verwendet und nach der Frameausgabe freigegeben; ihre Vorbereitung kann mehr
Speicher brauchen. Schrift-/Dichtewechsel verwerfen alle Editorpläne vor dem
Schließen der Fonts. Die Texturlebensdauer wird weiterhin erst nach der
Kommandorausgabe beendet.

`sb_ui_edit_geometry` liefert geliehene Quellzeilen, Glyphen und Cursorpunkte nur
während eines Callbacks. Der Aufrufer wählt ausdrücklich Original- oder IME-
Anzeigegeometrie. Glyphenmaße bleiben Backing-Pixel; vertikale Positionen sind
lokale Fensterpunkte. Der Callback darf Fonts und Cache nicht verändern.

Die vollständige Vorbereitung sehr langer Einzelzeilen und Änderungen großer
Dateien sind weiterhin Leistungsarbeit. Ein Layoutfehler erhält die Quelle und
meldet den Fehler; für noch nicht abgenommene Ressourcengrenzen bleibt der
bisherige Zeichnungs-/Eingabeweg als Rückfall erhalten. Dies ist kein Nachweis
flüssiger Bearbeitung jeder möglichen 16-MiB-Datei.

## Prüfung und verbleibender Umfang

Eine separat festgelegte Schrift-/Scriptaufteilung formt eine Referenz aus
Latein, Ligaturen/Akzenten, Hebräisch mit Ziffern, Arabisch und Emoji. Die
integrierte Eingabe wird bei 100/150/200 Prozent tatsächlich gerastert und
pixelgenau mit der Referenz verglichen, einschließlich einer logisch
zusammenhängenden, visuell getrennten Auswahl. Maus, Ligatur-Cursor,
RTL-Pfeile, vertikaler Wechsel, CRLF, gezeichnete Float-Cursorrechtecke und
Schriftgrößenwechsel an Richtungsgrenzen werden zusätzlich geprüft.

Die vorhandene Eingabe-/Undo-/Kapazitätsprüfung bleibt erhalten. Die IME-Prüfung
prüft nun den tatsächlich verwendeten Anzeigeglyphenplan und zusätzlich den
unveränderten Eingabeanker beim Markieren. Sie ersetzt keine echte japanische,
koreanische oder andere native Eingabemethode.

Weiter offen: einfache Labels/Schaltflächentexte und ihre allgemeine Bidi-
Anbindung, native Zeichenrechtecke, volle Unicode-Wortnavigation, große reale
Dateien, weitere Schrift-/DPI-/Gerätefälle sowie menschliche Screenreader-
und Eingabemethodenabnahme. Der Schritt schließt diese Release-Arbeiten nicht ab.
