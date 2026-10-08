# Gemeinsamer geformter Zeilenplan

Stand: 8. Oktober 2026. Der neue C-Plan verbindet die geprüfte
[Bidi-Absatzanalyse](BIDI.md) mit echten Glyphenpositionen, Schriftbereichen und
gemeinsamer Grundlinie. Er ist aktuell im UI-Prüfbuild angebunden. Der produktive
Renderer, der Editor und die nativen Textrechtecke benutzen ihn noch nicht.

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
Alle Maße verwenden die Backing-Pixeldichte der geladenen Fonts; die spätere
UI-Anbindung muss sie genau einmal in Fensterpunkte umrechnen.
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

## Weiterhin umzusetzen

Scriptzuordnung, Font-Fallback, Stilbereiche und endgültiger Umbruch müssen
diesen Plan gemeinsam benutzen; die sichtbare Darstellung muss seine Glyphen
zeichnen. Aus derselben Geometrie folgen visuelle Carets, Klickzuordnung,
Auswahl, IME und native Zeichenrechtecke. Dazu gehören Ligatur-/Graphempositionen
und Cursor-Affinität an Richtungsgrenzen. Absatz-/Zeilencaches benötigen
geprüfte Schlüssel, Font-/Dichte-Invaliderung und begrenztes Speicherbudget.
Reale Eingabe- und Screenreader-Abnahme bleibt ein eigener Release-Schritt.
