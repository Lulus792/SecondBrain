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

## Verhalten des Kompositionsschritts

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
kurze Texteingabe behielt in 0.9.25 ihren bisherigen Nuklear-Ereignisweg; große Text-Ereignisse
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


## Geordnete gewöhnliche Eingabe ab 0.9.26

Die gezielte Nachprüfung von 0.9.25 reproduziert auch ohne IME eine falsche
Feldzuordnung: normale Eingabe, Tab und weitere Zeichen im selben SDK-
Ereignispaket gelangen gemeinsam in das zweite Feld. Die neue Feldbindung
verwendet deshalb den vollständigen Übernahmeweg für alle aktiven Textfelder.
Fokus-/Caretaktionen werden vor nachfolgender Eingabe im Layout umgesetzt;
bereits bestätigte Zeichen werden vor einer solchen Aktion übernommen.

Druckbare Tastenevents und ihre Texte werden gemeinsam verarbeitet. Ein neuer
Burst-Test prüft acht Zeichen nach einer ersten Eingabe in höchstens zwei
Frames. Es wird kein zusätzlicher Frame pro Buchstabe verlangt. Weitere
Desktop-Folgen prüfen Tab vor Eingabe und eine Pfeilbewegung vor neuer Eingabe.

Eine zusätzliche negative Probe reproduziert fehlenden kurzen Frame-Takt beim
Öffnen eines Formulars. Befehle merken folgende Texte vor; nach dem Anwenden des
Befehls wird zuerst die geänderte Ansicht aufgebaut und dann das neue Feld
verwendet. Die Folge Formular-Shortcut und sofortiger Text prüft dieses Verhalten.
Die endgültige lokale Abnahme ist im Umsetzungsstand ergänzt.


Die vollständige Mausprüfung ergänzte zwei Korrekturen: Textfelder behalten
ihren aktiven Zustand unabhängig vom Eingabegerät; der visuelle Tastaturrahmen
bleibt getrennt. Beim Schriftwechsel verwendet die Einstellungskarte ihre
konkreten Zeilenhöhen und skaliert vorhandene Messungen bei proportionaler
Breite. Die Mausprüfung mit zwei aufeinanderfolgenden Schriftvergrößerungen
besteht wieder (126 Desktopassertions); die abschließende lokale Gesamtprüfung ist abgeschlossen.

Negative Ausgangsnachweise: build/ordinary-input-baseline.log (falsches Feld),
ordinary-input-burst-baseline.log (unnötige Einzel-Frames),
ordinary-form-baseline.log (fehlender schneller Neuaufbau),
ordinary-mouse-diagnostic.log (inaktives angeklicktes Feld) und
ordinary-scale-debug.log (versetztes Ziel nach Schriftwechsel).

Die finale Gesamtprüfung wurde nach erfolgreichen UI-Prüfungen durch die
volle Host-Festplatte unterbrochen (Bild-/CTest-Logdateien konnten nicht
geschrieben werden). Alte generierte Accessibility-/Interaktions-Testläufe
wurden gezielt bereinigt; die drei neuesten je Fixture bleiben erhalten.
Projektgedächtnisse und Rückfallpakete bleiben erhalten. Rund 19 GiB wieder
frei. Die fehlgeschlagenen und noch nicht ausgeführten Prüfungen wurden mit
unverändertem Quellstand erneut ausgeführt und bestehen (35/35, 55,95 s). Bereinigungsprotokoll:
build/generated-test-cleanup.json.

Nach Abschluss bestehen alle 51 Prüfungen des unveränderten Quellstands:
die ersten 16 UI-Prüfungen vor dem Platzmangel und die restlichen 35 danach.
Gezielte ASan/UBSan-Prüfung besteht mit 36 Kompositions-, 7 Ausschnitts- und
119 Navigationsassertions einschließlich 48 Cache-Rastervergleichen. Externe
Bibliotheken und Kern sind dabei nicht instrumentiert, macOS-Leaks ausgeschaltet.
Logs: build/ordinary-input-{complete-tests,resumed-tests,sanitizer-*}.log.
Native 0.9.26- und neue Paketabnahme folgen.
