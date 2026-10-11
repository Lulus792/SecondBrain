# Stilgrenzen mit dem Text halten

Stand: 11. Oktober 2026. Umsetzung ab 0.9.57; Paketabnahme folgt.
Die Änderung ergänzt den [Reader-Inline-Plan](READER_INLINE_PLAN.md).

## Befund

Zwei serielle Metal-Proben der 0.9.56 teilen die native Blockausgabe weiter auf.
In 68 Scroll-/Kamerabildern fallen etwa 151–153 ms für die erneute
Graphem-Normalisierung der Stilspannen an, also rund 2,2 ms pro Bild. Das
Erzeugen/Kopieren der Zeichenrechtecke kostet darin etwa 64–68 ms, das
Kopieren der Absatztexte etwa 9 ms. Die Stilberechnung ist in diesem Teilumfang
die größte Wiederholung. Die Messbereiche sind verschachtelt, Timer haben
eigenen Aufwand; ihre Summen sind keine unabhängige Frame-Gesamtdauer.

Originale: `build/native-stage-verification/`, Diagnosequellen/-buildscript,
`probe-1.log`, `probe-2.log`, zwei CSV-Dateien und unveränderte Profilkopie.

## Gehaltene Daten

Ein gehaltenes Inline-Ergebnis kann jetzt auch seine graphemgebundenen
Stilspannen behalten. Sie folgen der vorhandenen Regel: Ein zusammengesetztes
Zeichen übernimmt den Stil seines ersten Scalars. Hervorhebung eines späteren
Combining-Zeichens oder ZWJ-Teils trennt das Zeichen nicht auf.

Die Berechnung bleibt dieselbe eigene C-Funktion. Wiederverwendung erfordert
denselben gehaltenen Dokumentknoten und dessen Text-/Stiladressen und Anzahl.
Die übergeordnete Byteprüfung und die vollständige Freigabe bei Quellenersatz
bleiben erhalten. Breite, Schrift, Scrollen und Darstellung benötigen keinen
neuen textgebundenen Stilplan. Schriftgröße, Zeichenrechtecke, Umbruch und
Positionen kommen weiterhin aus der aktuellen Darstellung.

Das vorhandene 16-MiB-Quellbudget zählt auch die zusätzlichen Eintragsfelder
und eine exakt große Kopie der Stilspannen. Der aktuelle Aufrufer besitzt seine
eigene Ausgabe; ihre Änderung oder Freigabe berührt den gehaltenen Plan nicht.
Bei fehlendem Platz oder nicht passendem Inline-Eintrag bleibt die vollständige
frische Ausgabe erhalten. Wenn nur die zusätzliche Cache-Kopie nicht angelegt
werden kann, bleibt ebenfalls die bereits berechnete Aufruferausgabe erhalten.
Dieser Schritt begrenzt weiterhin gehaltene Daten, nicht die gesamte
Speicherspitze der Anwendung.

Tabellenzellen, Literaltexte und andere UI-Texte verwenden ihren vorhandenen
Pfad. Die Glyphen-Vorbereitung hat ihre eigene Stilberechnung; sie wird hier
nicht übersprungen. Die erste Vorbereitung und die vollständige Leistungs-
und Geräteabnahme bleiben offen.

## Prüfungen

177 direkte Quell-/Stilprüfungen testen Wiederverwendung, geänderte Quellen,
Referenzen, Budgetrückfall und unabhängige Ausgaben. Eine absichtlich veränderte
Aufruferausgabe verändert die spätere Cache-Ausgabe nicht. Explizite Unicode-
Fälle erhalten `é` und `👩‍🔬` als ganze Zeichen mit dem Stil ihres ersten
Scalars, auch bei Markdown-Stilwechseln innerhalb dieser Zeichen. Ein nicht
passender Knoten erhält die vollständige Normalisierung ohne Cache. Weitere
reale Inline-Einträge füllen das kleine gemeinsame Testbudget, bevor die
zusätzliche Stilkopie eines bereits gehaltenen Absatzes angefordert wird.
Alle 120 Stilspannen bleiben in der Aufruferausgabe erhalten, während der
Plan diese zusätzliche Kopie wegen der Budgetgrenze nicht speichert.

50 produktive Readerprüfungen erfassen mindestens 200 wiederverwendete
Absatz-Stilpläne pro unverändertem Fixture-Bild. Der Vergleich enthält Text,
Geometrie, Stilfelder und Schriftgröße. Resize, Schriftwechsel, Bearbeiten und
Retargeting bleiben im Prüfumfang. Gezielte Reader-Geometrie-, Interaktions-
und native Mac-Prüfung bestehen. Quell-/Reader- sowie 621 Mac-Assertions
bestehen auch unter haltendem ASan/UBSan: 39 eigene C-Dateien, 55 aktuelle
Quell-/Include-Hashes. Externe UI-Bibliotheken sind nicht instrumentiert;
kein Leak-/ThreadSanitizer- oder menschlicher Screenreader-Nachweis.

Originale: `build/reader-style-verification/` und
`build/reader-style-sanitizers/`. 78/78 lokale Release-Prüfungen bestehen
(685,84 s); die danach erweiterte Budgetprüfung besteht zusätzlich im Release-
und Sanitizer-Prüfer. Der Produktcode blieb dabei unverändert. Paket- und
Installationsnachweise folgen im [Umsetzungsstand](STATUS.md); installiert ist
noch 0.9.56.

## Alternierender Leistungsvergleich

Drei serielle Paare vergleichen die eingefrorene Desktop-/Quellimplementierung
aus `4a87e9f` mit der neuen Fassung. Timingprüfer, übrige UI-Bibliotheken und
Profilkopie sind gleich; beide Builds verwenden die aktuelle öffentliche
Desktop-Struktur mit zusätzlichen Zählern. Kein paralleler Build oder weiterer
UI-Prüfer. macOS 14.6.1/23G93, Intel-Mac, Metal, 1336×840.
Alle 18 Dateien der Profilkopie bleiben unverändert.

| Paar | Ruhe-Layout vorher → danach | Scroll-Layout vorher → danach | Kamera-Layout vorher → danach | Kamera-Frame-p95 vorher → danach |
| --- | ---: | ---: | ---: | ---: |
| 1 | 7,62 → 4,15 ms | 7,81 → 6,94 ms | 7,49 → 4,95 ms | 33,70 → 32,35 ms |
| 2 | 7,32 → 4,83 ms | 7,91 → 5,58 ms | 7,57 → 5,15 ms | 33,74 → 33,30 ms |
| 3 | 8,12 → 4,99 ms | 7,80 → 5,73 ms | 7,68 → 5,18 ms | 33,84 → 32,68 ms |

Median der paarweisen Layoutverbesserung: 38,62% im Ruhezustand, 26,54% beim
Scrollen, 32,55% bei Kamerabewegung, 9,31% beim Wechseln. Die Wechsel-Frame-p95
verbessert sich nicht in jedem Paar. Das erste Bild liegt in der neuen Fassung
bei 88–111 ms und wird im ersten Paar langsamer. GPU/Bildausgabe und Systemlast
bleiben relevant; die Zahlen belegen keine feste Bildrate oder vollständige
Flüssigkeit. Originale: `paired-profiles.json`, sechs CSV-/Textprotokolle und
`baseline/` unter `build/reader-style-verification/`.

Aktuelle Windows-/Linux-/ARM64-, Geräte-, Langzeit-, IME- und assistive Abnahmen
sowie SBUI-074 bleiben offen. 1.0 wird erst nach Nutzerfreigabe gesetzt.
