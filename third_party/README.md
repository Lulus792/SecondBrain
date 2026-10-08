# UI Abhängigkeiten von SecondBrain

Diese Abhängigkeiten werden ausschließlich in das UI-Ziel eingebunden. Der
C-Kern enthält keine externen Bibliotheken.

Ab 0.9.34 verwendet die formatierte Leseansicht das
[Bidi-Absatzlayout](ui/bidi18/README.md) mit SheenBidi 3.0.0 und aktualisierten
Unicode-18-Bidi-Daten. SheenBidi wird ausschließlich mit der UI verbunden.
Die originale Apache-2.0-Lizenz liegt unter `licenses/SheenBidi.txt` und wird
mitgeliefert; Quelle und Hash stehen im Bidi-Manifest.
Ein eigener [Glyphenplan](../docs/GLYPHENGEOMETRIE.md) nutzt den neuen,
hashgeprüften C-Hook `ui/ttf_shape.h` / `ui/ttf_shape.inc` in SDL_ttf 3.2.2.
Er ergänzt kontextgebundene Glyphenpositionen; die übrigen Textfunktionen bleiben
unverändert. Der Originalquelltext SHA-256
`25a42804b18809e5c4b2eb8ed787701551d0c680aff774b7d8c54486c0d42d38`
wird mit einem markierten Include erweitert. Bekannte vorbereitete Quellen
werden wiedererkannt, fremde Änderungen zurückgewiesen. Hook: eigener Code MIT;
SDL_ttf: vorhandene mitgelieferte zlib-Lizenz. Keine neue Runtime-Bibliothek.

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
- Einfügen zeigt einen dünnen Caret auch innerhalb einer Zeile; der Überschreibmodus
  behält seine Blockdarstellung. Die Cursorfarben folgen dem eigenen Blink-/Eingabezustand.
- Aktive editierbare Felder melden Cursor- und Cliprechteck aus ihrem Zeichnungslayout
  über `NK_TEXTEDIT_CARET_CUSTOM`; die eigene SDL-Anbindung positioniert daran native
  Eingabevorschläge. Der Hook verändert weder Text noch Auswahl.
- Ab 0.9.25 trennen Prepare-/Display-/Marked-Hooks die bestätigte Eingabe von
  einer vorläufigen, unterstrichenen Kompositionsdarstellung. Die Anzeige verwendet
  einen eigenen Textpuffer; Original und Undo bleiben bis zur Bestätigung erhalten.
- Ab 0.9.25 behalten Textkommandos Float-Geometrie für lange Zeilen. Sehr große
  ungerundete Textauswahlen werden vor dem Packen geclippt; vertikale
  Cursor-Sprünge stellen direkt die sichtbare Zeile her. Die eigene
  SDL_ttf-Anbindung rastert abgeschnittene vollständige Schriftläufe als
  sichtbare Ausschnitte. [Nachweise](../docs/IME.md).
- Ab 0.9.34 behalten auch Bildkommandos Float-Geometrie, damit geformte
  Textzeilen beim Scrollen keine Ganzzahlsprünge erhalten.
- Bei rückwärts belegtem Zeichenpuffer entsteht vor einer nötigen Vergrößerung
  kein überlaufender Probezeiger. Die Interaktionsprüfung führt diesen Wachstumspfad
  mit ASan/UBSan aus.

Das Undo-Protokoll bleibt begrenzt: 256 Operationen und 32.000 gespeicherte
Unicode-Zeichen. Große Änderungen können ältere beziehungsweise zu große
Undo-Einträge verdrängen. Die UI-Regression prüft ausdrücklich große Cursorpositionen
und verhindert eine Verwechslung von Zeichen- und Byte-Längen.

Der SDL-Renderer berücksichtigt ab 0.9.24 nur tatsächlich aktive Popups beim
Textfokus; ein geschlossenes Popup verdrängt das Elternfeld nicht mehr.
Ab 0.9.0 erlaubt ein bedingter
Nuklear-Hook (`NK_DRAW_TEXT_CUSTOM`) die Darstellung geformter Textläufe
als UI-Texturen; ohne Hook bleibt der Originalkonverter aktiv. app/ui.c ergänzt die Einfügefunktion,
verarbeitet mehrteilige Texteingaben vollständig und übersetzt Command-Tasten
unter macOS in die passenden Editieraktionen.

## AccessKit

Die [C-Bindings 0.23.1](https://github.com/AccessKit/accesskit-c/releases/tag/0.23.1)
werden als unverändertes, vorgebautes UI-Paket geladen. SHA-256:
`35b7ca8a6f1e038b5da35e1e9e5a0adaed9bfcf21e1496d29598fbbadcc7043f`.
Die Bibliothek ist intern in Rust implementiert; unsere Anbindung verwendet C.
macOS verwendet die statische Release-Bibliothek; unter Windows wird die UI-DLL
neben der Anwendung ausgeliefert. Ab 0.9.13 wird diese DLL aus festgelegter
Quelle mit einer [FindText-Ergänzung](../docs/UIA_TEXTSUCHE.md) gebaut. Linux baut
die festgelegte UI-Bibliothek aus Quelle mit Cargo/Rust ab 1.87 und einer Korrektur von zwei Cache-Signalaufrufen.
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
Ab 0.9.6 ergänzt dieselbe C-Anbindung `accessibilityRows` und die entsprechende
Selektorfreigabe für native Dokumenttabellen anhand ihrer tatsächlichen Kinder.
Diese Ergänzung wird auch ohne das neuere Überschriftenrollensymbol aktiviert.
Die festgelegten Windows-/Linux-Provider bieten noch keine UIA-/AT-SPI-Matrix-
schnittstelle; [Tabellenvertrag und Prüfgrenzen](../docs/TABELLEN.md).

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

## Geformter Text ab 0.9.0

Die C-Anbindung in app/text.c verwendet ausschließlich für die UI
[SDL_ttf 3.2.2](https://github.com/libsdl-org/SDL_ttf/releases/tag/release-3.2.2).
Archiv-SHA-256: `63547d58d0185c833213885b635a2c0548201cc8f301e6587c0be1a67e1e045d`.
Die statisch eingebundenen UI-Abhängigkeiten entsprechen den tatsächlichen
Submodulrevisionen dieses Tags:

- FreeType: SDL-Fork 9973564cfa63763a3e4ac67c09147899539b1e07, Archiv-SHA-256
  `026a05a49d114a1235d2926f4c03a9330e4b1a6efe7c217ec9607904c32907d4`.
  Verwendet wird die FreeType-Lizenzalternative FTL. Portions of this software
  are copyright © 1996–2023 The FreeType Project (www.freetype.org). All rights reserved.
- HarfBuzz: SDL-Fork 564bf9818a18709776856533829c0c04950773d6, Archiv-SHA-256
  `a448dd6c22d8e1e1cf39438c662251c1f97f810b8780eed4a6d6ada948c99ddc`.
  Old MIT sowie die separate Microsoft-MIT-Notiz zum USE-Anteil.

Originaltexte und besondere Hinweise (FreeType BDF/PCF/zlib, HarfBuzz USE)
liegen unter licenses/ und werden mit den Paketen ausgeliefert. Optionale externe
Kompressionsbibliotheken und das SDL_ttf-SVG-Emoji-Backend sind deaktiviert. HarfBuzz
benötigt beim Bauen einen C++-Compiler; der eigene Code bleibt C17. Diese
UI-Ziele sind vollständig vom separat baubaren fachlichen C-Kern getrennt.

Zusätzliche unveränderte Noto-Schriften (Arabisch, Hebräisch, Devanagari,
Symbols2) stammen aus derselben noto-fonts-Revision wie die bisherigen
Schriften. Noto Sans CJK JP Regular stammt aus
[noto-cjk/Sans2.004](https://github.com/notofonts/noto-cjk/tree/523d033d6cb47f4a80c58a35753646f5c3608a78),
Revision 523d033d6cb47f4a80c58a35753646f5c3608a78. Die separate Original-Lizenz
liegt unter assets/fonts/OFL-CJK.txt. Glyphenformen dieser CJK-Schrift folgen
der japanischen Variante. Andere regionale Varianten und umfassende
Schreibrichtungs-/Editorabnahme bleiben eigene Arbeiten.

### Schriftprüfsummen

- NotoSansArabic-Regular.ttf: `ceea25b464a656dc3b26849bab9356740401af62aedf1bfa8b7f0d9b75925b1b`.
- NotoSansHebrew-Regular.ttf: `a7fa16fffb27bedb060a0866267c29e9859aeb9c21cc33f5b3aaf6eb062eca85`.
- NotoSansDevanagari-Regular.ttf: `385e78e6359a9d88a0f243d53b1209d7548361ba2194e2b9ec779bcaa7e8949d`.
- NotoSansSymbols2-Regular.ttf: `882d142b9a1ef3fd7fa4225dbe95c10fab6664206eb4964c8ff705a4f6d02988`.
- NotoSansCJKjp-Regular.otf: `68a3fc98800b2a27b371f2fb79991daf3633bd89309d4ffaa6946fd587f375b5`.

## AT-SPI-Ebenen in 0.9.2

Der 0.9.1-Lauf findet im nativen Linux-Test die fehlende `level`-Eigenschaft.
accesskit_atspi_common 0.21.0 übernimmt sie noch nicht aus der gemeinsamen
Schema-Eigenschaft. Die eigene vorbereitete UI-Korrektur ergänzt den
AT-SPI-Attributwert mit natürlicher Ebenenzählung (`level + 1`, geprüft auf
Überlauf). Crate-SHA-256:
`52c182f9c282ac9c5638d876d551d15e5f7d397ec263349a0c6a2b61595dd5e4`.
Originaldatei src/node.rs SHA-256:
`8fafcc4f13a061cc46ea070a7b4e240027487f8f5507d75c449253c95136e4cf`;
korrigiert: `32f8e038ed152668c190a3acd672485b60e0e9c9abd017fc746bf3d502c47dd1`.
Unbekannte Quellen werden abgewiesen, erneute Vorbereitung ist bytegleich.
Cargo behält Versionen und Abhängigkeitsgraph der bestehenden Lockdatei;
nur diese weitere UI-Quelle wird lokal ersetzt. Lizenzen und Autoren bleiben
erhalten. Typprüfung des korrigierten Crates besteht lokal; native Linux-
Wiederholungsabnahme folgt und bleibt eine eigenständige Prüfung.


## Unicode-Daten

Ab 0.9.8 implementiert eigener C-Code die erweiterten Graphemgrenzen nach UAX #29,
Unicode 18.0.0. Die festgelegten Eigenschafts- und Testdaten liegen unter
[unicode](unicode/README.md); Hashes, Herkunft und Erzeugung stehen dort.
Es wird keine externe fachliche Bibliothek eingebunden. Abgeleitete Tabellen
und Originaldaten stehen unter der mitgelieferten Unicode License V3; der eigene
Algorithmus unter der Projektlizenz. Der Lizenztext ist auch in der App lesbar.
Nuklear erhält optionale eigene C-Hooks für Cursorgrenzen, vollständige Auswahl
und atomisches Texteingeben; diese Anpassungen sind bei einem Update zu erhalten
oder anhand der Unicode- und Editorprüfungen erneut zu ersetzen.


## Noto Emoji ab 0.9.9

Unveränderte variable Mono-Schrift aus Google Fonts, Commit
8b0a1d0f5983c89bc2b93f1b5fb55f9e252744b5, lokal NotoEmoji-Variable.ttf.
Die originale SIL-OFL steht unter licenses/OFL-Emoji.txt und gehört in App und
Paket. Quelle, Hashes und Anbindung: [Emoji-Vertrag](../docs/EMOJI.md).
Die Schrift ergänzt die UI; Originalzeichen werden nicht in Bilder umgeschrieben.


## Zeichenreferenzen und Mathematikschrift ab 0.9.14

[WHATWG-Daten](whatwg/README.md) erzeugen eine eigene C-Tabelle; keine neue
fachliche Bibliothek. Originalquelle, Hash und vollständige CC-BY/BSD-Lizenz
sind zugeordnet; WHATWG.txt gehört in App und Paket. Noto Sans Math ergänzt
vollständige Mathematik-Grapheme, unverändert aus Google-Fonts-Commit
823468bd7825152bce2b8fd2cf740432ad2fce8d. Font-SHA-256:
`3f495fe933c06786e4d5f6d86b8ee70b6753a68ee3b9d87528726de0f6e2c47d`.
Original-OFL unter assets/fonts/OFL-Math.txt und licenses/OFL-Math.txt, SHA-256
`403a95275b469061b7d4371c328e0ada3bc7d63328abe2e88aad5cd243b2fe21`.
[Vertrag und Nachweise](../docs/ENTITIES.md).

## Geprüfte Unterabhängigkeiten ab 0.9.20

Die [Inventur](../docs/LIZENZ_INVENTUR.md) dokumentiert 113 Cargo-Komponenten
(20 macOS, 24 Windows, 90 Linux) aus den festgelegten Quellen.
[Manifest](license-manifest.json) und Original-Sammlungen unter licenses/
halten Ursprung und Hashes fest. Dazu gehören weitere SDL3-/HarfBuzz-
Copyrightblöcke, YUV-BSD und die gewählte HIDAPI-BSD-Alternative.
Rust-Standardbibliothek/Compilerlaufzeit und Systemanteile bleiben gesonderte
offene Abnahmen; die Sammlung ersetzt diese Prüfung nicht.

Die [Rust-Laufzeitsammlung](../docs/RUST_RUNTIME_NACHWEIS.md) ergänzt ab 0.9.22
20 zugeordnete Registry-Versionen, compiler-builtins, stdarch und Quellhinweise
der beiden geprüften Compilerstände. Original-Archive und einzelne Payloads
sind durch Hashes zugeordnet. Abweichende Toolchains und die vollständige
Zuordnung tatsächlich gelinkter SDK-/Systemanteile bleiben offen.

Die eigene Windows-UI-Ergänzung für native Tabellen liegt in
`ui/accesskit_windows_table.rs` und `ui/accesskit_windows_table_patterns.rs`.
Sie steht unter der eigenen MIT-Lizenz, ergänzt ausschließlich den
festgelegten UI-Adapter und erhält dessen Original-Lizenzen und Lockfile.
Vorbereitung und Prüfvertrag: [Tabellen](../docs/TABELLEN.md).

Die Linux-Tabellenergänzung liegt in `ui/accesskit_atspi_table.rs`,
`ui/accesskit_atspi_table_filter.rs` und `ui/accesskit_unix_table.rs`.
Sie ergänzt ausschließlich die festgelegten externen UI-Adapter, unter
der eigenen MIT-Lizenz; ursprüngliche Lizenzen und Cargo-Versionen bleiben
erhalten. Der [Tabellenvertrag](../docs/TABELLEN.md) beschreibt den nativen
Baum, die Schnittstellen und die getrennten Ausführungsnachweise.
