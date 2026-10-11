# Parallele Schriftfreigabe

Stand: 12. Oktober 2026. Korrektur ab 0.9.71; eigener MIT-lizenzierter UI-Patch
an der festgelegten SDL_ttf-3.2.2-Quelle. Keine neue Bibliothek.

## Befund und Auswirkung

Der Linux-Debug-Lauf zu 0.9.69 / 8d74c8d scheitert in
`background-text-preparation` mit SegFault; 84 der 85 Tests bestehen.
[CI38184984352](https://github.com/Lulus792/SecondBrain/actions/runs/38184984352),
Job 114605268214. Windows Debug/Release und Linux Release einschließlich
entpackter Release-Pakete bestehen. Die palettenbezogenen Tests bestehen auch
im fehlgeschlagenen Linux-Job. Für den dortigen Absturz gibt es keinen Stacktrace;
seine genaue Ursache ist deshalb noch nicht abschließend nachgewiesen.

Bei der Nachprüfung der festgelegten UI-Quelle zeigt sich ein konkreter
Nebenläufigkeitsfehler: `TTF_OpenFontWithProperties` sperrt `TTF_state.lock`
für `FT_Open_Face`, `TTF_CloseFont` ruft `FT_Done_Face` aber ohne diese Sperre auf.
FreeType verwaltet dabei eine gemeinsame Schriftliste. Private Schriftobjekte
je Worker beseitigen diese gemeinsame Bibliotheksstruktur nicht.

FreeTypes [Bibliotheksvertrag](https://freetype.org/freetype2/docs/reference/ft2-library_setup.html#ft_library)
verlangt bei einer geteilten Bibliothek eine Sperre für Erzeugung und Freigabe
von Faces. Derselbe Hinweis steht in der tatsächlich eingebundenen
FreeType-Headerdatei. [SDL_ttf CloseFont](https://wiki.libsdl.org/SDL3_ttf/TTF_CloseFont)
verbietet gleichzeitig fremde Zugriffe auf dasselbe Fontobjekt. Unsere Worker
behalten jeweils eigene Fonts; die fehlende gemeinsame Bibliothekssperre ist
ein zusätzlicher Fehler. Quellen am 12. Oktober gelesen.

## Reproduktion und Korrektur

`tests/test_font_lifetime.c` öffnet und schließt 1024 private Fonts über acht
Worker. Barrieren richten die Freigaben aufeinander aus; jeder Font wird auf
seinem erzeugenden Thread verwendet und geschlossen. Die Bibliothek wird vor
Start der Worker initialisiert und erst nach allen Joins beendet.

Mit der bisherigen SDL_ttf-Quelle läuft der Test auf Intel/macOS 14.6.1 in das
60-Sekunden-Limit. Das währenddessen erhobene Prozessprofil zeigt ausschließlich
den Hauptthread in `TTF_Quit → FT_Done_FreeType → FT_Done_Library`; die Worker
sind bereits beendet. Das ist ein reproduzierbarer Bibliotheksfehler, keine
wartende Testbarriere. Originale: `build/contrast-lifetime-verification/font-baseline.log`
und `font-baseline-sample.txt`.

`third_party/ui/ttf_face_lifetime.inc` legt nun dieselbe `TTF_state.lock` um
`FT_Done_Face`. Die übrige Aufbereitung und Schriftformung bleiben parallel.
Die CMake-Vorbereitung prüft vollständige Hashes der Originalquelle sowie beider
älteren und der neuen vorbereiteten Fassung. Unbekannte Inhalte werden ohne
Dateiänderung abgewiesen; bekannte alte Fassungen werden gezielt aktualisiert.
Der Patch ist als eigener Eingriff gekennzeichnet; Originallizenz bleibt erhalten.

## Ausgeführte Prüfung und Grenzen

Mit der Korrektur besteht derselbe Paralleltest in 0,42 s. Acht gezielte lokale
CTest-Prüfungen bestehen in 22,42 s: Schriftlebensdauer, Quellvorbereitung mit
sechs Fällen, Hintergrund-/Reader-Vorbereitung, Palette/Systemdarstellung sowie
CLI-/Desktop-Version. Original: `build/contrast-lifetime-verification/font-fixed-tests.log`.

ASan/UBSan instrumentiert hier zusätzlich die betroffene externe UI: 49
C-Kompilationseinheiten aus Test, SDL_ttf und FreeType. SDL und HarfBuzz sind
nicht instrumentiert; Leakerkennung ist aus, kein TSan. Der korrigierte
1024er-Test besteht. In einer getrennten Vergleichsfassung wurde ausschließlich
die neue Freigabesicherung entfernt: ASan meldet `heap-use-after-free` in
`FT_List_Find → FT_Done_Face → TTF_CloseFont`, freigegeben von einem anderen
Worker; der Prozess endet mit SIGABRT. Die produktive vorbereitete Quelle wurde
für diesen Vergleich nicht geändert. Originale: `build/font-lifetime-sanitizers/`
mit `sources.json`, `fixed.log`, `baseline/run.log` und `baseline/result.json`.

Die neue Linux-Debug-Nachprüfung und Windows-/Linux-/Mac-Pakete stehen noch aus.
Die Korrektur allein schließt den früheren Linux-CI-Absturz nicht als verifiziert
behoben. Allgemeine Geräte-, Langzeit-, Screenreader- und Releaseabnahmen bleiben
offen; 1.0 wird nicht automatisch gesetzt. Installiert bleibt vorerst 0.9.69.
