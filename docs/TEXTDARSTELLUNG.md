# Schriftabdeckung und geformter Text

Stand: 7. Oktober 2026, Entwicklungsschritt 0.9.0. Die lokale Abnahme dieses neuen
Schritts besteht im unten beschriebenen Umfang; native Plattformabnahme folgt. Der fachliche C-Kern wird dabei nicht verändert.

## Grundlage vor der Änderung

Apple HIG [Typography](https://developer.apple.com/design/human-interface-guidelines/typography),
öffentliche Dokumentationsdaten am 7. Oktober erneut gelesen: lesbare Größen,
Prüfung im Nutzungskontext und erhaltene Hierarchie bei vergrößertem Text.
Die konkrete Textbibliothek und Ersatzschriften sind eigene plattformübergreifende
Entscheidungen, keine Apple-Vorgaben.

Die bisherige Nuklear-Glyphenliste deckt vorwiegend Latein, Griechisch und
Kyrillisch sowie bestimmte Symbole ab. Nicht enthaltene Zeichen erscheinen als
Ersatzglyphen; einzelne Glyphen ergeben außerdem keine kontextuelle Verbindung
arabischer Buchstaben. Das beeinflusst Notizen, Namen, Suche und Quellen gleichermaßen.

## Neue UI-Anbindung

SDL_ttf 3.2.2 formt Text mit HarfBuzz und rastert Schriften mit FreeType.
Alle Bibliotheken sind ausschließlich mit den UI-Zielen verbunden; eigener
Anwendungscode bleibt C17. Beim Bauen der UI benötigt HarfBuzz einen C++-Compiler.
Versionen, Quellen und SHA-256 sind in [third_party](../third_party/README.md)
und cmake/Text.cmake festgehalten. Der Kern bleibt separat mit C baubar.

Die UI misst Texte über dieselbe Schriftbibliothek, die sie zeichnet.
Nuklear behält Layout, Bedienelemente, primitive Geometrie und Editor; sein
Textkonverter ruft einen kleinen eigenen Hook auf. Geformte Textläufe werden
in unveränderter Zeichenreihenfolge der UI-Kommandos als Texturen eingefügt.
Die eigene Sternkarte und Glasdarstellung bleiben erhalten.

Noto Sans und Mono erhalten Noto-Ersatzschriften für Arabisch, Hebräisch,
Devanagari, Symbols2 und CJK. Die CJK-Schrift umfasst auch Kana und Hangul;
ihre regionalen Glyphenformen entsprechen der japanischen Variante.
Ersatzschriften verwenden denselben Schriftgrad wie die Hauptschrift, damit
unterschiedliche Fontmetriken keine ungewollt winzigen Buchstaben erzeugen.
[TTF_AddFallbackFont](https://wiki.libsdl.org/SDL3_ttf/TTF_AddFallbackFont).

Ein Zwischenspeicher hält weiße Texttexturen, deren Farbe beim Zeichnen gesetzt
wird. Schriftrolle, Text und verfügbare Breite gehören zum Schlüssel. Nicht mehr
verwendete Einträge werden nach dem Zeichnen entfernt; nach Überschreiten von
32 MiB wird der Cache geleert. Texturen eines laufenden Frames bleiben bis zu
seiner Ausgabe erhalten. Bis zu 1024 verschiedene Textläufe werden pro Frame
gehalten; breite Läufe sind auf 8192 Rasterpixel begrenzt. Dies ist noch kein
belastbarer Großdaten-/Mehrmonitor-Leistungsnachweis.

## Abnahme und verbleibende Arbeit

Der neue Test prüft kontextuelle arabische Verbindung gegenüber Einzelzeichen,
CJK-/Hangul-Abdeckung statt Fragezeichen, kombinierte Akzente und 100/150/200
Prozent Schriftgröße. Er zeichnet echte C-App-Texturen für die Bildprüfung.
Mit dem bisherigen Renderer scheitert derselbe C-Test an der fehlenden
arabischen Verbindung (Exit 1). Tests und Bilder werden jeweils konkret im
[Umsetzungsstand](STATUS.md) dokumentiert.

Diese Grundlage schließt die Textarbeit nicht ab. Gemischte Schreibrichtungen,
Scriptwechsel innerhalb einer Zeile, regionale CJK-Varianten, weitere Schriften,
Emoji, graphemgenaue Cursor-/Auswahlbedienung, IME-Position und native
Zeilengeometrie benötigen eigene Umsetzung und Abnahme. Das Vorhandensein
von HarfBuzz allein belegt diese Abläufe nicht. Unbekannte Glyphen können
weiterhin als Ersatzzeichen erscheinen; die gespeicherten UTF-8-Dateien
bleiben dabei vollständig erhalten. Menschliche Screenreader- und reale
Eingabeabnahme auf allen drei Systemen bleiben offen.


## Graphemgrenzen ab 0.9.8

Eigener C-Code verwendet die festgelegten Unicode 18.0-Daten für vollständige
Zeichen beim Bewegen, Auswählen und Löschen. Eine Texteingabe bildet einen
Undo-Vorgang; Kapazitätsfehler erhalten Inhalt und Auswahl. Alle Eingabefelder
verwenden dieselben Nuklear-Hooks; native Editor-Auswahl wird ebenfalls
begrenzt. [Vertrag, Quellen und Nachweise](GRAPHEME.md).

Die logische Einheit schließt gemischte Schreibrichtungen, kontextuelle
Wortgrenzen, exakte Maus-/Caret-Pixelgeometrie, IME-Kandidatenpositionen und
native Zeichenrechtecke noch nicht ab. Emoji-Fontabdeckung ist eine eigene
Aufgabe; eine korrekt erhaltene Sequenz belegt keine passende Glyphenanzeige.
