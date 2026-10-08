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


## Vollständige Schriftläufe ab 0.9.9

Eigene C-Schriftwahl ersetzt die bisherige reine SDL_ttf-Fallback-Anbindung.
Ein vollständiges Graphem erhält eine abdeckende Schrift; benachbarte kompatible
Zeichen werden gemeinsam geformt. Noto Emoji ergänzt konkrete verbundene Emoji-
Prüffolgen. Metriken und Rasterung verwenden dieselbe Auswahl und eine gemeinsame
Grundlinie. [Quellen, Hashes, Abnahme und Grenzen](EMOJI.md).

Das Zusammensetzen folgt bisher der Quellreihenfolge. Ein vollständiger Bidi-
Absatzalgorithmus, exakte visuelle Eingabe-/Mausgeometrie und IME-/native
Zeichenrechtecke bleiben offen. Ganze Unicode-Grapheme in der Eingabe und
richtige Glyphen für die einzelnen geprüften Folgen ersetzen diese Arbeit nicht.

Am 7. Oktober erneut geprüfte Grundlage für den nächsten Schritt:
[UAX #9, Unicode 18.0.0, Revision 52](https://www.unicode.org/reports/tr9/tr9-52.html)
trennt logische Speicherung, Absatzauswertung und visuelle Zeilenreihenfolge.
[SDL_ttf-Richtung](https://wiki.libsdl.org/SDL3_ttf/TTF_SetFontDirection) steuert
die Formung eines Laufs. Die festgelegte HarfBuzz-Dokumentation
(docs/usermanual-what-is-harfbuzz.xml, Abschnitt Bidirectionality) verlangt
vorbereitete Richtungsabschnitte. Eigene Folgerung: Schrift-Fallback allein kann
diese Absatzaufgabe nicht lösen; native und visuelle Auswahl benötigen eine
gemeinsame Abbildung zwischen logischen Indizes und dargestellten Läufen.
Dies ist Recherche für die verbleibende Umsetzung, kein neuer Funktionsnachweis.


Ab 0.9.11 verwenden [Inline-Stile](INLINE_STILE.md) getrennte Schriftkopien
und gemeinsame Grundlinien. Grapheme über Formatgrenzen bleiben beim Zeichnen
eine Einheit. Native Stilattribute und die oben genannten Bidi-/Geometriearbeiten
bleiben offen.


## Native Eingabeanker ab 0.9.24

Am 8. Oktober 2026 erneut geprüft: [SDL_SetTextInputArea](https://wiki.libsdl.org/SDL3/SDL_SetTextInputArea)
verwendet Fensterkoordinaten und einen relativen horizontalen Cursoroffset;
[SDL_GetTextInputArea](https://wiki.libsdl.org/SDL3/SDL_GetTextInputArea) liest diese
Werte zurück. Vorschlagsfenster können daran ausgerichtet werden. Das tatsächliche
Verhalten einer Eingabemethode benötigt zusätzlich eine native Bedienabnahme.

Die eigene Anbindung übernimmt den Caret aus dem Zeichnungslayout jedes aktiven,
editierbaren Nuklear-Felds. Das gilt für Editor, Suche und Formularfelder. Der
Anker folgt Cursor, Auswahlende, Scrollposition und Schriftgröße; er bleibt im
sichtbaren Cliprechteck. Fensterpunkte verhindern eine doppelte Retina-Skalierung.
Inaktive, verdeckte und schreibgeschützte Felder liefern keinen neuen Anker;
ohne aktives Feld wird der alte Bereich gelöscht. Unveränderte Koordinaten
verursachen keinen wiederholten nativen API-Aufruf pro Frame.

Die neue Regression fragt echte SDL-Fensterwerte nach Einfügen, Pfeiltasten,
Auswahl, Zeilenwechsel, Scrollen, 150 Prozent Schrift, einzeiligem Formularfeld
und Fokusverlust ab; eine echte Popup-Folge prüft den verdrängten Elternanker
und zurückkehrenden Textfokus. Lokale Prüfungen bestehen: 49/49 und nach letzter
Popup-Korrektur 3/3 (134 Editorassertions). Der native Plattformnachweis folgt
im Umsetzungsstand. Dieser Schritt implementiert keine Preedit-Komposition,
keine Bidi-Abbildung und keine neue native Zeichenrechteck-Schnittstelle.
