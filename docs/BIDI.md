# Gemeinsame Schreibrichtungsgrundlage

Stand: 8. Oktober 2026. Der neue C-Absatz-/Zeilenplan ist lokal geprüft, aber
noch nicht mit dem produktiven Renderer oder Editor verbunden. Die installierte
App bleibt 0.9.33 / d48ee9bf7075. Dies ist ein Umsetzungsschritt für die offenen
Textaufgaben, kein Nachweis fertiger gemischter Schreibrichtungen in der App.

## Originale und Entscheidung

[UAX #9, Unicode 18.0.0, Revision 52](https://www.unicode.org/reports/tr9/tr9-52.html)
unterscheidet logische Speicherung, Absatzauswertung und visuelle Reihenfolge
nach dem Zeilenumbruch. HarfBuzz benötigt bereits aufbereitete Richtungsläufe;
einzelne Schrift-Fallbacks lösen diese Absatzaufgabe nicht.

[SheenBidi 3.0.0](https://github.com/Tehreer/SheenBidi/releases/tag/v3.0.0)
stellt eine C-Implementierung bereit. Die Ausgabe enthält Unicode-17-Daten.
Für die eigene UI-Grundlage wurden ausschließlich die Bidi-Klassifikation und
Klammer-/Spiegelpaarung aus unveränderten Unicode-18-Originalen neu erzeugt.
Quellarchiv, Originale und Ausgaben sind im
[Manifest](../third_party/ui/bidi18/manifest.json) mit SHA-256 festgelegt.
Die übrigen Bibliotheksfunktionen für Script/Kategorie bleiben auf dem
Originalstand, sind keine Unicode-18-Schnittstelle und werden nicht exponiert.

Die Bibliothek dient ausschließlich Textlayout. Eigener Kern und eigene
Fachfunktionen bleiben C ohne externe Bibliotheken. Aktuell wird die Grundlage
nur bei UI-Prüfbuilds kompiliert; Anwendung und Runtime-Pakete verlinken sie
noch nicht. Vor der sichtbaren Anbindung werden auch Original-Lizenz und
Herkunftshinweise in Paket und eingebauter Lizenzansicht ergänzt.

## C-Vertrag

[app/bidi.h](../app/bidi.h) bietet einen eigenen, undurchsichtigen Absatztyp.
Er besitzt eine unveränderliche UTF-8-Kopie, meldet den nach P1 verbrauchten
ersten Absatz einschließlich Separator und ermittelt Grundrichtung und Levels.
Leere Absätze behalten die gewünschte Grundrichtung. Ungültiges UTF-8,
überlange Eingaben und ungültige Richtungswerte werden zurückgewiesen.

Eine logische Zeile wird anschließend nach L1/L2 in visuell von links nach rechts
geordnete Läufe zerlegt. Deren Bytebereiche bleiben in logischer Quellreihenfolge;
ungerade Levels kennzeichnen RTL. Umbruch erfolgt vorher im gemeinsamen Layout.
Zeilen müssen vollständig in einem Absatz liegen und auf Unicode-Zeichengrenzen
beginnen und enden. Die spätere Layoutintegration muss zusätzlich Graphemgrenzen
erhalten. Wiederholte Zeilenauswertung verändert keine Absatzlevels. Quellen
werden weder umsortiert noch durch Spiegelzeichen ersetzt; Spiegelung gehört
zur Formung der sichtbaren Glyphen.

Speicher gehört dem Absatz beziehungsweise der Ergebniszeile und wird explizit
freigegeben. Ausgaben müssen vor Wiederverwendung freigegeben werden. Fehler
werden als SBStatus weitergereicht. Die Analyse ist durch die bestehende
16-MiB-Textgrenze begrenzt; zusätzliche Skalierungs-/Langzeitabnahme bleibt offen.

## Zwei bei der Einbindung entdeckte Fehler

Die öffentliche UTF-8-Zeilenfunktion von SheenBidi 3.0.0 kann bei der Folge
`S RLE PDI R`, Richtung LTR, einen Lauf mitten im UTF-8-Zeichen PDI beenden.
Reproduktion: BidiTest.txt, Zeile 208322. Mit UTF-32 tritt dies nicht auf.
Die eigene Anbindung arbeitet deshalb intern in skalaren Zeichenpositionen
und übersetzt erst fertige Zeilenläufe über eine explizite Tabelle in UTF-8-
Bytebereiche. Die offizielle Prüfung benutzt weiterhin die eigene UTF-8-API
und prüft jede Laufgrenze und vollständige Zeichenabdeckung.

Unicode 18 ordnet U+221D den Spiegelpartner U+1DB10 zu. Der originale Generator
speichert Differenzen als int16_t und schneidet den Abstand 112883 ab.
Der geprüfte Generatorschritt verwendet dafür 32 Bit, auch in den erzeugten
C-Differenzen und beiden Lookupfunktionen. Die Paarungstests prüfen positive
und negative Richtung einschließlich aller Positionen ohne Spiegelpartner.

## Tatsächliche lokale Prüfung

macOS/Intel, AppleClang, Release:

- 91.707 Fälle aus dem originalen BidiCharacterTest.txt.
- 490.846 Klassenfolgen aus BidiTest.txt, mit 770.241 Richtungsfällen.
- Zusammen 861.948 normative Richtungsfälle: Grundrichtung, Post-L1-Levels,
  L2-Reihenfolge, originale UTF-8-Bytes und skalare Laufgrenzen.
- Alle 438 Spiegelpaare und alle 1.114.112 Unicode-Positionen geprüft.
- Eigene Fälle für leeren Text, ungültige Eingaben/Ranges, CRLF-Absatzverbrauch,
  unveränderliche Kopie, Zeilenteilbereiche und nicht mutierte Absatzlevels.
- 36.859.513 Assertions; CTest `unicode18-bidi-layout` besteht in 1,96 s.
- Derselbe Bestand unter AddressSanitizer und UndefinedBehaviorSanitizer besteht.
  Instrumentiert: eigener Bidi-Code, C-Prüfer und gesamte Bidi-Bibliothek.
  Verlinkte bestehende Kernfunktionen sind nicht instrumentiert; macOS-Leakprüfung
  ist abgeschaltet. Dies ist kein Sanitizer-Nachweis für die gesamte App.
- Erneute Generierung aus frisch entpacktem Originalarchiv liefert dieselben Bytes.
- Separater Build mit `SB_BUILD_UI=OFF`: 32/32 Kernprüfungen bestehen in
  16,32 s, ohne die neue UI-Bibliothek einzubinden.

Generator: `python3 tools/make_bidi_data.py --archive ARCHIV --check` mit einem
CMake-Pfad über `--cmake`, falls nötig. Dieser Entwicklerweg baut den originalen
UI-Datengenerator mit C++; normaler Build und Anwendung benötigen ihn nicht.
Windows-/Linux-Nachweise dieses neuen Schritts folgen erst nach tatsächlichen
CI-Läufen. BidiCharacterTest prüft L3/L4 und P1 ausdrücklich nicht; eigene
CRLF-/Spiegelprüfungen ergänzen einen Teil dieser Lücke.

[Lauf 37740652510](https://github.com/Lulus792/SecondBrain/actions/runs/37740652510)
bestätigt die neue Prüfung in Linux Debug/Release und macOS ARM Debug/Release
sowie Intel Release. Die drei entsprechenden entpackten Release-Pakete
bestehen. Windows erreicht wegen umgewandelter Repository-Zeilenenden die
neue Tabellen-Hashprüfung nicht erfolgreich; dies ist ein Configure-Fehler,
kein bestandener Windows-Layouttest. Explizite Git-Dateiregeln erhalten jetzt
die LF-Ausgabe der erzeugten C-Tabellen und die Originalbytes der Unicode-
Dateien. Tatsächlich ausgeführte Git-Checkoutfilter mit `core.autocrlf=true`
bestätigen alle zwölf Eingabe-/Tabellenhashes. Native Windows-Abnahme folgt
mit dem korrigierten Quellstand.

## Verbleibende Integration

Die gemeinsame Absatzanalyse muss vor Schrift-, Script- und Stilaufteilung
stehen und über umgebrochene Zeilen erhalten bleiben. Dieselben endgültigen
Glyphenpositionen müssen Zeichnung, Messung, Klickziele, Caret, visuelle
Pfeilnavigation, Auswahl, IME und native Zeichenrechtecke versorgen. Dafür
bleiben Renderer, Absatzcache, Nuklear-Eingabe und native Provider anzubinden
und mit echten Mischtexten in allen Eingabefeldern und der Leseansicht zu prüfen.
Ein bestandener Absatzalgorithmus ersetzt diese Arbeit nicht.
