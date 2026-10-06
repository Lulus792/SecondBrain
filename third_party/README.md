# UI Abhängigkeiten von SecondBrain

Diese Abhängigkeiten werden ausschließlich in das UI-Ziel eingebunden. Der
C-Kern enthält keine externen Bibliotheken.

## SDL3

SDL3 3.2.30 wird statisch gebaut oder aus einer vorhandenen passenden
Entwicklungsinstallation eingebunden. Originalquelle:
[Release 3.2.30](https://github.com/libsdl-org/SDL/releases/tag/release-3.2.30).
Das Quellarchiv wird beim UI-Build bei Bedarf geladen; sein SHA-256 ist im
CMake-Build festgelegt. Lizenz: zlib. Die Original-Lizenz ist unter
licenses/SDL3.txt enthalten und wird mit jedem Anwendungspaket ausgeliefert.

## Nuklear

Die Header unter ui/ stammen aus dem
[Originalprojekt](https://github.com/Immediate-Mode-UI/Nuklear/tree/9f7750296f176e506c2b24ff55bc24495e2db750),
Revision 9f7750296f176e506c2b24ff55bc24495e2db750. Verwendet wird die MIT-Alternative
der mitgelieferten Nuklear-LICENSE. Die Quellen sind im Repository enthalten.

Lokale Anpassungen des Nuklear-Headers:

- UTF-8-Einfügen arbeitet mit Byte-Längen und Unicode-Zeichenpositionen.
- Vor einer Auswahlersetzung wird Speicher reserviert; bei ungültigem Text oder
  fehlender Kapazität bleibt die bisherige Auswahl erhalten. Diese Korrektur
  wurde aus einer vorhandenen MIT-lizenzierten UI-Anpassung übernommen.
- Rückgängig und Wiederholen stellen eine zusammengefallene Auswahl am Cursor her.
- Positionen und Einfügelängen jenseits des 16-Bit-Bereichs werden erhalten.

Das Undo-Protokoll bleibt begrenzt: 256 Operationen und 32.000 gespeicherte
Unicode-Zeichen. Große Änderungen können ältere beziehungsweise zu große
Undo-Einträge verdrängen. Die UI-Regression prüft ausdrücklich große Cursorpositionen
und verhindert eine Verwechslung von Zeichen- und Byte-Längen.

Der SDL-Renderer-Header bleibt unverändert. app/ui.c ergänzt die Einfügefunktion,
verarbeitet mehrteilige Texteingaben vollständig und übersetzt Command-Tasten
unter macOS in die passenden Editieraktionen.

## AccessKit

Die [C-Bindings 0.23.1](https://github.com/AccessKit/accesskit-c/releases/tag/0.23.1)
werden als unverändertes, vorgebautes UI-Paket geladen. SHA-256:
`35b7ca8a6f1e038b5da35e1e9e5a0adaed9bfcf21e1496d29598fbbadcc7043f`.
Die Bibliothek ist intern in Rust implementiert; unsere Anbindung verwendet C.
macOS verwendet die statische Release-Bibliothek; unter Windows wird die UI-DLL
neben der Anwendung ausgeliefert. Linux baut die festgelegte UI-Bibliothek aus
Quelle mit Cargo/Rust ab 1.87 und einer Korrektur von zwei Cache-Signalaufrufen.
Die Toolchain ist nur zum Bauen dieser UI-Abhängigkeit erforderlich.

Die C-Binding-Quelle ist Commit 8b6ed37c20ed4c59390e253407983333053662ba
(0.23.1), Archiv-SHA-256 f15581c841eed0f2f6cec6a6f9b7fd4ca9d34a654546efa22ae29351efa06568.
Der Linux-Adapter ist accesskit_unix 0.24.0, Crate-SHA-256
202f24df034a7476d07b7f74284de84f6d62aabd858dbe7ee9cad3b7ad6f8f9d.
Seine AddAccessible-/RemoveAccessible-Strukturen werden als einzelnes D-Bus-
Argument übergeben; vorher fehlte die äußere Ebene. CMake prüft den Original-
bzw. korrigierten Quellhash, bevor die Anpassung erfolgt. Cargo verwendet die
festgelegte Lockdatei mit unverändertem Abhängigkeitsgraph; nur diese UI-Quelle
wird lokal ersetzt. Die C-Regression liest tatsächliche Signale und GetItems.
Die ursprünglichen Lizenz-/Autorenhinweise bleiben im Paket.
Der Kern bleibt unabhängig und ohne externe Bibliotheken baubar.

Die eigene macOS-C-Anbindung gleicht die vom Adapter gelieferte Heading-Rolle an
NSAccessibilityHeadingRole an, wenn die Systemkonstante vorhanden ist. Die
unveränderte 0.23.1-Bibliothek liefert dort noch die Zeichenfolge Heading. Die
Korrektur verwendet Objective-C-Runtime-APIs im App-Prozess und verändert keine
Dateien der Bibliothek. Sie wird nur an der AccessKit-View-Klasse und an AccessKitNode
angebracht; ältere Systeme ohne die Konstante behalten den Adapterwert. Der
native macOS-Test vergleicht mit der tatsächlichen Systemkonstante. Diese
Versionsanpassung ist bei einer Änderung der UI-Abhängigkeit erneut zu prüfen.

Unveränderte MIT-, Apache-2.0- und Chromium-BSD-Lizenztexte sowie AUTHORS liegen
unter licenses/AccessKit-* und gehören zu jedem Anwendungspaket. Native Adapter
sind integriert; die Grenzen der Abnahme stehen im
[Zugänglichkeitsstand](../docs/BARRIEREFREIHEIT_PLAN.md).

## Schriftasset

Noto Sans Regular und Noto Sans Mono Regular stammen aus
[noto-fonts](https://github.com/notofonts/noto-fonts/tree/ffebf8c1ee449e544955a7e813c54f9b73848eac),
Revision ffebf8c1ee449e544955a7e813c54f9b73848eac. Die Schrift und ihre SIL Open Font
License sind unter assets/fonts enthalten. Es werden keine Apple-Schriftassets
weiterverteilt.


## Linux-Systemdarstellung

Die UI-Anbindung von Systemvorgaben verwendet auf Linux die dynamischen
Systembibliotheken libdbus-1 und GIO/GLib. Sie werden ausschließlich mit dem
Desktop-Ziel verbunden und nicht in den fachlichen Kern eingebunden.
D-Bus liest das Settings-Portal; GIO liest unter GNOME verfügbare Einstellungen.
Sie werden nicht im Paket als eigene Bibliothekskopien ausgeliefert. Herkunft:
[D-Bus](https://www.freedesktop.org/wiki/Software/dbus/),
[GLib/GIO](https://gitlab.gnome.org/GNOME/glib). Voraussetzungen und Grenzen stehen
in [Distribution](../docs/DISTRIBUTION.md) und [Einstellungen](../docs/EINSTELLUNGEN.md).
