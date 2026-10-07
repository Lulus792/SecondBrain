# Emoji-Schrift für die Lumen-Oberfläche

Quellen am 7. Oktober 2026 vor der Anbindung eingesehen:
[Noto Emoji](https://github.com/googlefonts/noto-emoji),
[SDL_ttf-Fallbacks](https://wiki.libsdl.org/SDL3_ttf/TTF_AddFallbackFont).
Eigene Gestaltungsentscheidung: Die monochrome Noto-Variante übernimmt die
gewählte Textfarbe und passt zu den zurückhaltenden Farben. Die Schrift wird
mitgeliefert; die normale Nutzung benötigt keine Systeminstallation.

Festgelegter Google-Fonts-Commit: `8b0a1d0f5983c89bc2b93f1b5fb55f9e252744b5`.
Original: https://github.com/google/fonts/tree/8b0a1d0f5983c89bc2b93f1b5fb55f9e252744b5/ofl/notoemoji.
Unveränderte Fontdatei lokal NotoEmoji-Variable.ttf, SHA-256: `de6c18832938afc99caf132b39d6a30a19bac7f2e812e28db2535b4608d27551`.
Originale OFL-Lizenz, SHA-256: `500bb1ccf43df7bbb522112f9133a52b16e1c35e809632f5d8609b179152de5b`.
Die Dateiumbenennung verändert keine Schriftinhalte. Das Original und seine
Copyright-Hinweise bleiben erhalten. Der Originaltext ist als 19. Lizenzressource in der App angebunden; die
Paketprüfung verlangt Schrift und Lizenz. Das fertige Paket wird separat geprüft.

Glyphenabdeckung, vollständige Emoji-Verbindungen, gemischte Textläufe und
Schriftgrößen müssen nach dem Anbinden tatsächliche Mess- und Rasterprüfungen
bestehen. Eine zusätzliche Schrift allein belegt diese Abläufe noch nicht.
Die Unicode 18-Zeichengrenzen sind eine separate bereits geprüfte Eingabegrundlage;
die Glyphenversion der Schrift ist kein pauschaler Unicode 18-Abdeckungsnachweis.


## Eigene C-Schriftwahl für vollständige Zeichen

Die erste reine Fallback-Anbindung zeigte wiederholte Einzelfiguren in verbundenen
Emoji. Die UI wählt nun anhand vollständiger Unicode-Grapheme eine Schrift,
die alle zugehörigen sichtbaren Zeichen abdeckt. Die Emoji-Schrift hat bei
passenden Emoji-/Präsentationsfolgen Vorrang. Benachbarte kompatible Zeichen
werden als gemeinsamer Lauf geformt; eine Schriftänderung verliert keine
Zeichen. Messung und Rasterung verwenden dieselbe Auswahl. Ober-/Unterlängen
bestimmen eine gemeinsame Grundlinie. ASCII-Abstände bleiben in der normalen
Textschrift. Originaltext und eigene Icons werden dabei nicht verändert.

Drei passende lokale Prüfungen bestehen (5,56 s), darunter 43 Text-, 111 Editor- und
58 Lizenzassertions. Textmessung prüft verbundene Frau/Laptop-, Familien-,
Flaggen- und Hauttonfolgen bei 100/150/200 % sowie Mono-Code. Hangul-Jamo und
vorgeformte Silbe haben dieselbe gemessene Breite. Raster vor/nach Schriftwahl
wurden betrachtet. Das ist noch keine allgemeine Bidi-/IME-/Geräteabnahme und
keine pauschale Abdeckung aller neu hinzugekommenen Unicode 18-Glyphen.

Fehler beim Messen, Löschen oder Zusammenfügen der Rasterflächen werden vor
dem Cache-Eintrag abgefangen; unvollständige Fehlerbilder werden nicht gespeichert.

## Verbleibende Grenzen

Schriftläufe werden in Quellreihenfolge zusammengesetzt. Das implementiert noch
keinen vollständigen Unicode-Bidi-Absatzalgorithmus. Maus-/Cursorrechtecke,
IME-Positionen und native Zeichenrechtecke benötigen eigene Abnahmen. Die
oben genannten Emoji sind konkrete Prüffälle, kein Nachweis für jede Sequenz.
