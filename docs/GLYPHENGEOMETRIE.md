# Gemeinsamer geformter Zeilenplan

Stand: 8. Oktober 2026. Der neue C-Plan verbindet die geprüfte
[Bidi-Absatzanalyse](BIDI.md) mit echten Glyphenpositionen, Schriftbereichen und
gemeinsamer Grundlinie. Ab dem Entwicklungsschritt 0.9.34 benutzt die formatierte
Leseansicht diesen Plan für Umbruch, Höhe und Rasterung. Editor, einfache
Bedienelementtexte und native Textrechtecke benutzen ihn noch nicht.
Die installierte App bleibt bis zur gesonderten Paketabnahme bei 0.9.33.

## Grundlage

Die festgelegte SDL_ttf-3.2.2-Quelle formt normale Textläufe mit HarfBuzz.
Ihre öffentlichen Größen-/Substring-APIs stellen jedoch keine gemeinsame
Absatzgeometrie über eigene Richtungs-, Schrift- und Stilgrenzen bereit.
Der neue C-Hook benutzt denselben HarfBuzz-Font und die vorhandenen
FreeType-Glyphenmetriken. Es wird keine weitere externe Bibliothek ergänzt;
eigener Kern und Fachfunktionen bleiben unabhängig davon.

Originale: [HarfBuzz-Buffer](https://harfbuzz.github.io/harfbuzz-hb-buffer.html),
[Unicode-Funktionen](https://harfbuzz.github.io/harfbuzz-hb-unicode.html),
festgelegter SDL_ttf-Quelltext `src/SDL_ttf.c`, Funktionen
CollectGlyphsFromFont und Find_GlyphByIndex. Der Hook ist eigener MIT-lizenzierter
UI-Code; Herkunft und Quellvorbereitung sind in [third_party](../third_party/README.md)
dokumentiert. Der Originalquelltext wird ausschließlich nach festgelegter
SHA-256-Prüfung erweitert.

## Quelltext, Zeile und Schriftbereiche

[app/shaped.h](../app/shaped.h) erhält einen schon analysierten, unveränderlichen
Absatz sowie eine logisch umgebrochene Zeile. Geordnete Schrift-/Stil-/Script-
Bereiche decken den Absatz vollständig ab. Ihre Grenzen und die Zeilenenden
dürfen vollständige Grapheme nicht trennen. L1/L2 stammt weiterhin aus diesem
Absatz, auch wenn eine Fortsetzungszeile mit einer anderen Schrift beginnt.

Der Plan besucht die Richtungsbereiche und ihre Schriftteile in visueller
Reihenfolge. Innerhalb eines RTL-Bereichs werden auch die Schriftteile von
rechts nach links abgearbeitet. HarfBuzz erhält die vollständige logische Zeile
als Kontext für das Teilstück. So bleiben arabische Verbindungen über einen
Stilwechsel erhalten, ohne den gespeicherten Text umzusortieren oder zu ändern.

Richtung und Script gelten nur für den jeweiligen Buffer. Der Hook verändert
keine Font-Einstellung und keinen bestehenden SDL_ttf-Textcache. Für Spiegelung
erhält der neue Buffer die geprüfte Unicode-18-Paarung aus dem Absatzplan;
der Standardbuffer und sein Unicode-Zustand bleiben erhalten.

Das Ergebnis hält Glyphenindex, Quellcluster, Schrift, Level, Advance,
Grundlinienposition und Rastermetriken sowie die geformten Teilbereiche.
Alle Glyphenmaße verwenden die Backing-Pixeldichte der geladenen Fonts;
die Leseansicht rechnet sie genau einmal in Fensterpunkte um.
Fontobjekte sind geliehen und müssen den Plan überleben. Ergebnisarrays werden
explizit freigegeben. Fehler lassen die leere Ausgabe erhalten. Geometrisch
wachsende Arrays und binäre Suche nach den betroffenen Schriftbereichen
vermeiden wiederholtes Kopieren beziehungsweise Vollscans aller Stile pro Lauf.

## Tatsächliche lokale Abnahme

Intel/macOS Release: 380 Assertions bei 18/27/36-Punkt-Fonts bestehen.
Geprüft sind arabisches Joining gegen dieselbe Glyphe im ganzen Wort,
ein bewusst isolierter Gegenfall, gemischtes Deutsch/Arabisch/Hebräisch,
RTL-Schriftteilreihenfolge, ein fetter Stil mitten im verbundenen arabischen
Wort, RTL-Absatzrichtung in einer nur lateinischen Fortsetzungszeile,
Graphemschutz, unzulässige Bereiche, leere Ausgaben und unveränderte Fontparameter.
Ligaturen/Combining sowie normale/fette arabische Glyphenmetriken werden gegen
tatsächlich geladene Glyphenbilder geprüft.

Eine bewusst abweichende Spiegelcallback-Funktion wirkt auf den Glyphenindex;
der Callback erhält außerdem U+221D → U+1DB10 aus Unicode 18. Dies bestätigt
die Datenanbindung, keine zusätzliche Fontabdeckung für U+1DB10.

Eine aus denselben Glyphenkoordinaten gerasterte Vorschau bei 36 Punkt wurde
betrachtet (`build/text-ui/shaped-layout.bmp`, lokal auch PNG). Sie zeigt
Deutsch, Hebräisch, verbundenes Arabisch mit Stilwechsel und lateinische
Ligaturen/Akzente. Das ist eine Vorschau des Zeilenplans, keine neue App-Ansicht.

AddressSanitizer/UndefinedBehaviorSanitizer: dieselben 380 Assertions bestehen.
Instrumentiert sind neuer C-Plan, Absatzanbindung, Bidi-Bibliothek, Prüfer und
die vollständige Hauptdatei SDL_ttf.c einschließlich Hook. Verlinkte SDL-,
HarfBuzz-/FreeType-, weitere SDL_ttf- und Kernobjekte sind nicht instrumentiert;
macOS-Leakprüfung ist abgeschaltet. Der Nachweis gilt für diesen Teilscope.

Neun lokale Release-Prüfungen bestehen (27,49 s): neuer Glyphenplan, normative
Bidi-Prüfung, bestehender Text-/Viewport-/Navigationscache, Grapheme, IME,
Lizenzressourcen und Runtime-Inventur. Die Quellvorbereitung erhält die bekannte
Originaldatei und eine schon vorbereitete Datei; geänderte und abgeschnittene
Quellen werden ohne weitere Veränderung abgewiesen. Neue native Plattform-
nachweise folgen über CI; lokale Mac-Ausführung ersetzt sie nicht.

## Einbindung in die formatierte Leseansicht ab 0.9.34

Die eigene Scriptzuordnung verwendet Unicode-18-Scripts, Script_Extensions und
BidiBrackets. Originaldaten und Hashes stehen in `tools/make_script_data.py`;
der Generator erhält bei `--check` die vorhandene Datei und vergleicht alle Bytes.
Die Zuordnung folgt einer eigenen UI-Regel nach
[UAX #24, Revision 41](https://www.unicode.org/reports/tr24/tr24-41.html):
Grapheme bleiben ganz, Common/Inherited berücksichtigen den Kontext und mögliche
Scriptwerte; schließende Klammern behalten das Script ihrer passenden öffnenden
Klammer. Absatzseparatoren setzen diesen Kontext zurück. Diese Anwendungspolitik
ist kein zusätzlicher normativer Unicode-Algorithmus.

Je Absatz werden Richtung, Schrift-/Stil-/Scriptbereiche und Graphemgrenzen einmal
vorbereitet. Bei Absätzen bis 65.536 Bytes liefert eine vollständige Formung kumulierte
Breiten als Umbruchschätzung. Jede gewählte Grenze wird danach mit eigenem
Zeilenkontext exakt geprüft und bei Bedarf korrigiert; Ligaturen und Joining
sind deshalb nicht an die Schätzung gebunden. Größere Absätze verwenden
schrittweise Wrap-Proben und binäre Grenzsuche. Normale Leerzeichen bieten
Wortgrenzen.
Umbruch und Zeichnung benutzen die endgültigen Glyphenpositionen derselben Zeile.
Auch nur lateinische Fortsetzungszeilen behalten die ursprüngliche Absatzrichtung.
`sb_ui_styled_geometry` liefert geliehene Zeilen samt Absatz-Quelloffset und
vertikaler Position in Fensterpunkten, ohne den Quelltext zu verändern.

Layoutpläne vergleichen Text, Stilbereiche, Font, Breite und Padding exakt;
ihre Arrays zählen zum 32-MiB-Budget. Absätze über 65.536 Bytes oder ohne Platz
werden frisch mit denselben Regeln berechnet. Das ist noch kein abschließender
Leistungsnachweis für große Einzelabsätze. Font-/Dichtewechsel verwerfen Pläne
vor dem Freigeben ihrer Schriften.

Der Rastercache speichert sichtbare Zeilenbilder anhand der tatsächlichen
Fonts, Glyphenindizes und Positionen. Große Bilder werden auf sichtbare Ausschnitte
begrenzt und gekachelt. Das Budget beträgt 32 MiB; die Freigabe erfolgt nach der
Ausgabe der Nuklear-Kommandos, damit deren Texturen gültig bleiben. Image-Kommandos
behalten Float-Geometrie statt Ganzzahlkürzung bei Scrollbewegungen. Farbige
Emoji-Schriften sind damit nicht gesondert abgenommen; die gebündelte Emoji-Schrift
ist monochrom.

Die neue Reader-Prüfung vergleicht tatsächliche Pixel mit einer separat festgelegten
Schriftaufteilung für Arabisch mit fettem Mittelteil neben Latein: drei Schriftgrößen,
links/mittig/rechts, Cache und frischer Plan. Die vorhandene Textprüfung prüft Stile
an den tatsächlich verwendeten Glyphenfonts statt an alten Textkommandogrenzen.
Prüfergebnisse und Plattformgrenzen stehen im [Umsetzungsstand](STATUS.md).

## Weiterhin umzusetzen

Editor, Suche, Formulare und einfache Labels benötigen die gemeinsame Geometrie.
Daraus folgen visuelle Carets, Klickzuordnung, Auswahl, IME und native
Zeichenrechtecke. Dazu gehören Ligatur-/Graphempositionen und Cursor-Affinität an
Richtungsgrenzen. Reale Eingabe-, Geräte-, Leistungs- und Screenreader-Abnahme
bleibt ein eigener Release-Schritt; die Reader-Integration schließt ihn nicht ab.
