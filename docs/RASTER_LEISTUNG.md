# Gleiche Glasbilder mit weniger Rechenarbeit

Stand: 11. Oktober 2026, Umsetzung in 0.9.48.

Die Kamerafahrt verändert den räumlichen Hintergrund und damit die Abtastung
unter den Glaskarten. Der eigene CPU-Renderer zeichnet diese Fläche neu.
Im alten Weg wurden auch im flachen Karteninneren die Rundungsdistanz,
Randbeleuchtung und Abtastposition für jeden Pixel berechnet.

Die Optimierung trennt ausschließlich das sicher flache Innere von Rand und
Rundung. Beide Achsen müssen dieselbe ursprüngliche Distanzbedingung erfüllen.
Abtastspalten werden einmal pro Karte berechnet, die Abtastzeile einmal pro
Innenzeile. Kontrast und deckende Darstellung füllen dort konstante Farben.
Die tatsächlichen Kanten, Schatten und Rundungen behalten ihren bisherigen Weg.
Die Kartenreihenfolge bleibt erhalten. Außerhalb des Rasters liegende Karten
erhalten keinen künstlichen Innenbereich.

Es entsteht kein zusätzlicher dauerhafter Cache. Die lokale Spaltentabelle hat
4096 Ganzzahlen, passend zur bestehenden Rastergrenze; die normale logische
Rastergröße bleibt auf1600×1100 begrenzt. Es werden keine Shader, weitere
Abhängigkeiten, reduzierten Effekte oder niedrigere Bildauflösungen eingeführt.

## Unabhängiger Bildvergleich

`tests/fixtures/space_v0947.c` ist die eingefrorene eigene 0.9.47-Implementierung
mit umbenannten Exporten (SHA-256:
`2962c87892536177d3dd87e53031f0e353ee5eb826d6e4b72f9d7353d2e970b5`).
Sie dient als Versionsreferenz und wird bei einer
Optimierung nicht zusammen mit der Implementierung geändert.
`test_space_raster.c` vergleicht vollständige RGBA-Bytes von Himmel, Szene und
Glasfläche sowie wiederholte Cacheaufrufe. Enthalten: sieben Fenstergrößen bis
3840×2160, hell/dunkel, deckend/transparent, hoher Kontrast, unterschiedliche
Mauspositionen, Weltpartikel, Sterne/Verbindungen, überlappende und abgeschnittene
Karten, Bruchteile und Radien sowie64 Flächen.1012 Assertions bestehen auch unter
haltendem ASan/UBSan im dokumentierten Teilumfang. Die erste Versuchsfassung
verletzt den Vergleich bei einer vollständig außerhalb liegenden Karte; die
leere Schnittfläche wird nun ausdrücklich übersprungen.

## Gemessener produktiver Ablauf

Drei abwechselnde Vorher-/Nachher-Läufe mit demselben Intel/macOS-Metal-Renderer
und einer unveränderten18-Dateien-Kopie des Projektgedächtnisses. Kamera und
Wechsel über Eingang, STATE und die62-KiB-Übergabe verwenden den produktiven
Desktop. Zahlen bezeichnen durchschnittliche Zeichenzeit je Phase, in ms.

| Paar | Kamera vorher | Kamera danach | Änderung | Wechsel vorher | Wechsel danach |
| --- | ---: | ---: | ---: | ---: | ---: |
| 1 | 12,58 | 10,15 | −19,32% | 14,24 | 12,57 |
| 2 | 21,80 | 17,58 | −19,36% | 23,23 | 19,01 |
| 3 | 23,06 | 12,73 | −44,80% | 20,34 | 12,95 |

Median der paarweisen Kamera-Zeichenverbesserung:19,36%. Systemlast und Present
schwanken; die vollständigen Frames enthalten weiterhin Layoutspitzen und
Wartezeit der Bildausgabe. Das ist keine allgemeine60-fps-Garantie oder neue
Windows-/Linux-/GPU-Matrixabnahme. Der originale Datenbestand und die
Profilkopie werden nicht bearbeitet. Protokolle/Parameter/Hashes:
`build/space-raster-verification/`, Speicherprüfung in
`build/space-raster-sanitizers/`. Ausgeführte Gesamtabnahme und Grenzen:
[STATUS](STATUS.md).
