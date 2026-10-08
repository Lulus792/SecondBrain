# Zusammengesetzte Texteingabe

Stand: 8. Oktober 2026. C-Umsetzung lokal geprüft; native Eingabemethodenabnahme
und vollständige Bidi-/visuelle Textgeometrie bleiben gesondert offen.

## Grundlagen vor dem Entwurf

Apples [Textfelder](https://developer.apple.com/design/human-interface-guidelines/text-fields)
wurden erneut anhand der öffentlichen Dokumentationsdaten gelesen: kurze Felder
und größere Textansichten unterscheiden, Feldzweck sichtbar halten und Fokus
logisch weitergeben. Die folgende portable Kompositionsanbindung ist eine eigene
Implementierungsentscheidung, keine Zusage einer nativen Apple-Komponente.

[SDL_TextEditingEvent](https://wiki.libsdl.org/SDL3/SDL_TextEditingEvent) liefert
vorläufigen UTF-8-Text und Auswahlpositionen in Unicode-Zeichen.
[SDL_TextInputEvent](https://wiki.libsdl.org/SDL3/SDL_TextInputEvent) liefert den
bestätigten Text. [SDL_IME_IMPLEMENTED_UI](https://wiki.libsdl.org/SDL3/SDL_HINT_IME_IMPLEMENTED_UI)
trennt eigene Kompositionsanzeige von nativen Kandidatenlisten.
[SDL_ClearComposition](https://wiki.libsdl.org/SDL3/SDL_ClearComposition)
beendet die Komposition ohne Abschalten der Texteingabe. Quellen und festgelegte
SDL-3.2.30-Header wurden am 8. Oktober geprüft.

## Verbindliches Verhalten dieses Schritts

Vorläufiger Text erscheint unterstrichen direkt an der Einfügestelle und ersetzt
in der Anzeige eine bestehende Auswahl. Der eigentliche Feldinhalt, Suchtreffer,
Speicherstatus und Undo-Verlauf bleiben bis zur Bestätigung unverändert. Die
Darstellung verschiebt den anschließenden Text; sie ist kein schwebender Tooltip.
Die Auswahl der Eingabemethode und der native Kandidatenanker folgen diesem Text.
Bestätigung übernimmt den Text vollständig als einen Undo-Vorgang. Ungültige
UTF-8-Daten, überschrittene Grenzen und Kapazitätsfehler erhalten den Originalinhalt.

Während einer Komposition gehören Texteingabe, Pfeile und Enter der Eingabemethode;
Enter darf weder ein Formular abschicken noch eine Editorzeile zusätzlich einfügen.
Escape verwirft ausschließlich die Komposition. Tab, Maus-Fokuswechsel und Verlust
des Fensterfokus beenden sie; vorläufiger Text wird dabei nicht gespeichert.
Feld-/Projektwechsel und geänderter Originaltext dürfen keine alte Komposition
in ein anderes Feld übertragen. Kandidatenlisten bleiben beim Betriebssystem.

## Nachweise

Lokaler Release-Neubau besteht mit 51/51 Prüfungen (292,04 s). Der eigene
Kompositionstest besteht mit 36 Assertions; die Desktop-Navigation mit 109
Assertions einschließlich 48 unveränderter Cache-Rasterfälle. Die neue Folge
Bestätigung → Tab → weitere Zeichen prüft zwei echte Formularfelder.
12.000 Eingabebytes werden vollständig übernommen; ein zu kleines Feld erhält
Inhalt und Auswahl. Der markierte Text ist als tatsächliches Raster betrachtet.

Bestätigte IME-Eingabe wird vor nachfolgenden Fokusaktionen übernommen.
Vorgemerkte Text-Ereignisse besitzen ihre UTF-8-Bytes selbst; Fokusaktionen und
nachfolgende Texte werden in Reihenfolge über Framegrenzen abgearbeitet.
Solche vorgemerkten Aktionen halten die App im kurzen Frame-Takt. Die gewöhnliche
kurze Texteingabe behält ihren bisherigen Nuklear-Ereignisweg; große Text-Ereignisse
verwenden den vollständigen Übernahmeweg. Native Provideraktionen warten auf eine
bereits bestätigte, noch nicht im Feld übernommene Eingabe.

Die Prüfung hat zwei bestehende native Fokuslücken gefunden: später gezeichnete
passive Panels und Nuklears Edit-Garbage-Collector bei geänderter sichtbarer
Feldanzahl. Der eigene stabile Textfokus wird nach dem Layout wiederhergestellt;
der erste Formularfokus aktiviert zugleich das zugehörige Fenster.

Weitere Logs und Plattformnachweise stehen im [Umsetzungsstand](STATUS.md). Eingespeiste SDL-
Ereignisse allein belegen noch keine tatsächliche japanische/chinesische/koreanische
Eingabemethode auf allen zugesagten Betriebssystemen.


## Lange Zeilen und Sanitizer-Nachprüfung

Die instrumentierte Prüfung fand zunächst einen tatsächlichen Überlauf der
16-Bit-Textrechteckbreite (72.158 Punkte). Der Eingabeinhalt war erhalten,
die Darstellung aber nicht zuverlässig. Textkommandos behalten jetzt ihre
logische Float-Geometrie. Sehr große rechteckige Auswahlen werden vor dem
Packen auf den sichtbaren Clip begrenzt; Unterstreichungen ebenfalls.

Der eigene Textcache rastert bei abgeschnittenen Läufen nur den sichtbaren
Ausschnitt, formt dabei aber den vollständigen kompatiblen Schriftlauf.
[TTF_CreateSurfaceTextEngine](https://wiki.libsdl.org/SDL3_ttf/TTF_CreateSurfaceTextEngine)
und [TTF_DrawSurfaceText](https://wiki.libsdl.org/SDL3_ttf/TTF_DrawSurfaceText)
sind APIs der bereits festgelegten SDL_ttf-UI-Bibliothek. Das fügt keine fachliche
Abhängigkeit hinzu. Font-Auswahl, gemeinsame Grundlinie und eigener Textcache
bleiben erhalten. Ausschnitt und Pixeloffset gehören zum exakten Cacheschlüssel;
Texturen bleiben auf 8192 Pixel pro Ausschnitt begrenzt. Ganze Bidi-Absätze,
visuelle Cursorgeometrie und Großdaten-/Mehrmonitor-Leistungsabnahme bleiben offen.

Die neue unabhängige Rasterreferenz vergleicht das sichtbare Ende einer Zeile
mit mehr als 65.535 Punkten Breite gegen eine kürzere Zeile und prüft zugleich,
dass tatsächlich Schriftpixel statt eines leeren Bilds erscheinen. Ausgewählte
und normale Varianten bestehen mit 7 Assertions. Der Editor folgt bei großen
vertikalen Sprüngen direkt bis zur sichtbaren Cursorzeile.

Gezielte ASan/UBSan-Instrumentierung von UI, Text-/Materialrenderer,
Desktop-Ereignisrouting und Tests besteht mit 36 Kompositions-, 7 Ausschnitt-
und 109 Navigationsassertions einschließlich 48 Cache-Rastervergleichen.
Kern und externe Bibliotheken sind in diesem Lauf nicht instrumentiert;
macOS-Leakprüfung ist ausgeschaltet. Logs: build/composition-tile-sanitizer-*.log.
