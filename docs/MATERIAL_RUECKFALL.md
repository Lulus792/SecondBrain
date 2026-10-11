# Lesbare Oberfläche bei einem Materialfehler

Stand: 12. Oktober 2026. Implementierung ab 0.9.67; vor der Freigabe bleibt
[RELEASE.md](RELEASE.md) maßgeblich.

## Fehler und Änderung

Wenn Rasterpuffer, Materialtextur, Upload oder Materialzeichnen scheiterten,
löschte die App den Hintergrund immer dunkel. Das widersprach der hellen
Darstellung. Bei einer fehlgeschlagenen Rasterzuweisung blieben außerdem bereits
angelegte Teilpuffer bis zum nächsten Versuch oder Beenden erhalten.

Der Rückfall verwendet nun die ausgewählte Hell-/Dunkel-/Kontrastdarstellung.
Er zeichnet die vorhandenen Karten als deckende gerundete Flächen mit begrenzten
Dreiecken auf dem Stack, ohne einen weiteren CPU-Rasterpuffer oder eine neue
Textur anzulegen. Dialoge verdecken weiterhin fremde Beschriftungen; der eigene
Inhalt wird danach gezeichnet. Der nächste erfolgreiche Materialversuch kehrt
zur normalen Glasdarstellung zurück. Teilzuweisungen werden bei einem Fehler
sofort freigegeben; Größe und Cachebereitschaft bleiben zurückgesetzt.
Der normale Rasterpfad und die Darstellungseinstellungen bleiben erhalten.

## Tatsächliche Prüfung

Ein Test kompiliert die aktuelle `app/space.c` mit eng begrenzter Interzeption.
Er lässt jede der drei Rasterzuweisungen sowie Materialtexturerstellung, Upload
und Texturzeichnen gezielt fehlschlagen. Andere UI-/SDL-Zuweisungen laufen normal.
Die Interzeption ist nur im Testprogramm, nicht in der ausgelieferten App.

108 Kombinationen aus drei Fenstergrößen, sechs Darstellungsmodi und sechs
Fehlerstellen bestehen: 1122 Bedingungen zu Fehlerrückfall, unverändertem
Vordergrund trotz Hintergrundbeschriftungen, sichtbarem eigenen Text, Karten-
und Himmelfarben, bereinigten Teilpuffern und Wiederherstellung. Ausgeführt
mit dem SDL-Software-Renderer und mit Metal auf Intel/macOS 14.6.1/23G93,
Retina-Dichte 2.00. Tatsächliche helle und dunkle Metal-Aufnahmen wurden angesehen.

Die normale Vordergrund-Materialprüfung besteht mit 352 Pixelbedingungen,
800×600, 1336×840 und 1920×1080, 100/200 % Schrift, sechs Darstellungsmodi;
sowohl Software als auch Metal. 1012 unabhängige CPU-Rasterbedingungen bestätigen
gleiche normale Sky-/Szenen-/Glas-/Cache-/Resizepixel zum eingefrorenen Original.
Die vorherige Fassung wurde tatsächlich negativ geprüft: Teilpuffer bleiben
vorhanden, und die helle Himmelfarbe stimmt im Rückfall nicht.

Die Bildprüfung liest den aktuellen Backbuffer jetzt vor `Present` und hält eine
CPU-Kopie für die Regionenvergleiche. Nach `Present` ist ein nativer Backbuffer
keine verlässliche Kopie des gerade gezeichneten Frames. Das korrigiert die
Prüfmethode; es ändert nicht die produktive Übergangstextur. Deren native
Pixelidentität ist weiter eine gesonderte Abnahme.

ASan/UBSan bestehen für dieselben 1122 Fehler-/Rückfall- und 352 normalen
Materialbedingungen. 38 eigene C-Dateien wurden instrumentiert, 60 aktuelle
Quell-/Includehashes bestätigt. Externe UI-Bibliotheken nicht instrumentiert,
Leakerkennung aus, kein ThreadSanitizer. Fünf weitere Prüfungen für
Systemdarstellung, Interaktion/Bewegung/Layout, Editor und beide Versionswege
bestehen (51,95 s). Kein neuer vollständiger lokaler 84er-Lauf. Sauberes Paket
und Installation folgen.
Originale: `build/material-failure-verification/negative.log`,
`negative-light.log`, `targeted.log`, `failure-recovery.log`, `native-normal.log`,
`native-failures.log`, die Metal-Aufnahmen und `tests/fixtures/space_faults.c`.

## Grenzen

Geprüft sind die genannten Materialoperationen, kein vollständiger Speichermangel
des Prozesses oder Verlust des GPU-Geräts. Nuklear-/Schrift-/Backendzuweisungen
können eigene Fehler haben; die externen UI-Bibliotheken werden nicht durch diese
Interzeption ersetzt. Windows/Linux/ARM64, reale Geräte, menschliche Bedienung
und die vollständige Release-Abnahme bleiben offen. 1.0 nur nach Nutzerfreigabe.
