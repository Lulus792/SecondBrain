# UI Abhängigkeiten von SecondBrain

Diese Abhängigkeiten werden ausschließlich in das UI-Ziel eingebunden. Der
C-Kern enthält keine externen Bibliotheken.

## SDL3

SDL3 3.2.30 wird statisch gebaut oder aus einer vorhandenen passenden
Entwicklungsinstallation eingebunden. Originalquelle:
[Release 3.2.30](https://github.com/libsdl-org/SDL/releases/tag/release-3.2.30).
Das Quellarchiv wird beim UI-Build bei Bedarf geladen; sein SHA-256 ist im
CMake-Build festgelegt. Lizenz: zlib. Die Lizenz liegt im Originalarchiv und muss
mit einem späteren Anwendungspaket ausgeliefert werden.

## Nuklear

Die Header unter ui/ stammen aus dem
[Originalprojekt](https://github.com/Immediate-Mode-UI/Nuklear/tree/9f7750296f176e506c2b24ff55bc24495e2db750),
Revision 9f7750296f176e506c2b24ff55bc24495e2db750. Verwendet wird die MIT-Alternative
der mitgelieferten Nuklear-LICENSE. Die Quellen sind im Repository enthalten.

Lokale Anpassungen des Nuklear-Headers:

- UTF-8-Einfügen arbeitet mit Byte-Längen und Unicode-Zeichenpositionen.
- Vor einer Auswahlersetzung wird Speicher reserviert; bei ungültigem Text oder
  fehlender Kapazität bleibt die bisherige Auswahl erhalten. Diese Korrektur
  stammt aus der vorhandenen, MIT-lizenzierten UI-Anpassung im Physim-Projekt.
- Rückgängig und Wiederholen stellen eine zusammengefallene Auswahl am Cursor her.
- Positionen und Einfügelängen jenseits des 16-Bit-Bereichs werden erhalten.

Das Undo-Protokoll bleibt begrenzt: 256 Operationen und 32.000 gespeicherte
Unicode-Zeichen. Große Änderungen können ältere beziehungsweise zu große
Undo-Einträge verdrängen. Die UI-Regression prüft ausdrücklich große Cursorpositionen
und verhindert eine Verwechslung von Zeichen- und Byte-Längen.

Der SDL-Renderer-Header bleibt unverändert. app/ui.c ergänzt die Einfügefunktion,
verarbeitet mehrteilige Texteingaben vollständig und übersetzt Command-Tasten
unter macOS in die passenden Editieraktionen.

## Schriftasset

Noto Sans Regular stammt aus
[noto-fonts](https://github.com/notofonts/noto-fonts/tree/ffebf8c1ee449e544955a7e813c54f9b73848eac),
Revision ffebf8c1ee449e544955a7e813c54f9b73848eac. Die Schrift und ihre SIL Open Font
License sind unter assets/fonts enthalten. Es werden keine Apple-Schriftassets
weiterverteilt.
