# SecondBrain: aktueller Stand

Stand: 8. Oktober 2026. Originale: [STATUS](../../docs/STATUS.md),
[Release-Aufgaben](../../docs/RELEASE.md), [Quellen](SOURCES.md).
Chronologie und Rückfallkopien: [Journal vom 8. Oktober](journal/2026-10-08.md).

## Produkt und installierte App

Eigene C17-App für Mensch und KI: Projekte/Notizen verwalten, lesen, bearbeiten,
suchen, archivieren und sichern; Quellen und gespeicherten Kontext kopieren.
Lumen-Sternkarte, Glaskarten/Icons, direkte Pfeilnavigation, Kamerafahrt,
weiches Scrollen, Startfokus, Textcursor und große Leseansicht sind implementiert.
Originalbytes, Entwürfe und erkannte Konflikte bleiben geschützt.

Installiert: **0.9.34, Build 4eda9dbfce37**, Intel/macOS Release.
Entpacktes Paket besteht mit 126/157/75 Assertions, zwei Prozessneustarts,
Produktions-CLI und Runtime-Importprüfung. Lizenzbündel exakt, eigenes Gedächtnis
beim Laden unverändert, installierte Ansicht betrachtet. Rückfallkopie:
`/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.33-20261008-104353`.

## Geprüfte Fortschritte

- Neue C-Bidi-Grundlage: 861.948 Unicode-18-Richtungsfälle,
  438 Spiegelpaare und alle Unicode-Positionen bestehen; ASan/UBSan im
  dokumentierten Teilscope und bytegenaue Neugenerierung ebenfalls.
  32/32 Kernprüfungen ohne UI bestehen. Ab 0.9.34 in der formatierten Leseansicht angeschlossen; [Vertrag/Nachweise](../../docs/BIDI.md).
- Geformter C-Zeilenplan erhält vollständigen Zeilenkontext, Glyphen-/Quell-
  positionen und RTL-Schriftteile. 380 Assertions bei drei Größen, Raster-
  vorschau, ASan/UBSan im Teilscope und neun Release-Nachprüfungen bestehen.
  Reader-Einbindung lokal geprüft; Editor/native Anbindung offen; [Vertrag](../../docs/GLYPHENGEOMETRIE.md).
- Korrigierte Bidi-CI zu 23837f1: alle acht nativen UI-Testschritte und vier
  entpackten Pakete bestehen, einschließlich Windows Debug/Release.
  19 Jobs erfolgreich; Intel-Debug scheitert erst am Artefakt-Upload mit
  GitHub-DNS-Fehler nach bestandenen Tests. Der folgende Glyphenplan zu
  1906c3d besteht alle 20 Jobs/vier Pakete; neue Reader-CI folgt separat.
- Erneute gezielte macOS-Release-Nachprüfung des Nutzerfeedbacks:
  135 Bewegungs-/Layout- und 144 Navigations-Assertions, darunter alle vier
  vorherigen Listen-/Leseansichten nach Leerung, erneutem Tippen und Escape.
  Beide Anlegekarten als aktuelle Screenshots betrachtet. Keine Änderung am
  installierten Produktbuild; [Details](../../docs/NAVIGATION_POLITUR.md).
- 0.9.28: GPU-Übergänge, Desktop-Startfokus/Projektabschluss, freies Suchende
  und erster stabiler Formularrahmen. 51 lokale Release-Prüfungen, gezielte
  UI-/Desktop-/Renderer-Sanitizer und [20 Plattformjobs](https://github.com/Lulus792/SecondBrain/actions/runs/37730572189)
  inklusive vier Pakete bestehen.
- Native Tabellenmatrix: Mac ab 0.9.27; Windows Grid/Table ab 0.9.29
  ([20 Jobs](https://github.com/Lulus792/SecondBrain/actions/runs/37732498154));
  Linux Table/TableCell ab 0.9.32
  ([20 Jobs](https://github.com/Lulus792/SecondBrain/actions/runs/37736031017)).
  Zellen, Header, Spannen, Eltern/Indizes und 200%-Ansicht sind im
  [Tabellenvertrag](../../docs/TABELLEN.md) getrennt nachgewiesen.
- 0.9.33: direkter Cache-/Parent-Index für markierte rechteckige C-Tabellen.
  16.384 Indizes, unabhängiger Child-Zugriff, falsche Metadaten und Defunct-
  Kontexte geprüft. Synthetischer Cache-Teilschritt 43,560 s → 48,186 ms;
  keine globale FPS-Aussage. Neue native Linux-Prüfung und
  [alle 20 Jobs/vier Pakete](https://github.com/Lulus792/SecondBrain/actions/runs/37737157648)
  bestehen.
- Geordnete normale/IME-Eingabe, vorläufige Komposition, UTF-/Graphemgrenzen,
  große Texte und Feldbindung sind implementiert. Verträge/Nachweise stehen
  in [IME](../../docs/IME.md), [Grapheme](../../docs/GRAPHEME.md) und STATUS.
- Importprüfung, originale Lizenzinventur/Rust-Laufzeitzuordnung, Support-/
  Beitragsregeln und Wartungsablauf sind umgesetzt; ihre dokumentierten
  Grenzen bleiben maßgeblich.

## Neue Reader-Integration (0.9.34)

Formatierte Leseansicht verwendet gemeinsamen Absatz-/Script-/Glyphenplan,
ganze Grapheme, Stil-/Schriftwahl und denselben Umbruch für Höhe/Rasterung.
Float-Bildpositionen erhalten Zwischenstände beim Scrollen. Lokal 56/56
Release-Prüfungen, 105 gezielte ASan/UBSan-Reader-Assertions, frischer UI-Build
mit BUILD_TESTING=OFF und 32/32 separat gebaute Kernprüfungen bestehen.
SheenBidi-Lizenz ist in App und Paketen integriert. Neue Plattform-/Paketabnahme
folgt gesondert für Windows/Linux/Apple Silicon; Intel-Mac-Paket und
Installation 0.9.34 bestehen. Native Metal-Messung zeigt
Scroll-Median 1,87 ms und Wechsel-Median 14,03 ms; erster langer Wechsel
bleibt mit 123,30 ms ein offener Ausreißer (zuvor 237,01 ms im neuen Plan). Details: STATUS/GLYPHENGEOMETRIE.

## Nächste Arbeiten

0.9.33 ist vollständig im dokumentierten automatisierten Umfang abgenommen.
Weitere Release-Arbeiten: Bidi/visuelle/native Textgeometrie und reale Eingabemethoden;
menschliche VoiceOver/NVDA/Orca-, Dialog-, Geräte-, Mehrmonitor-/Langzeitabnahme;
volle Windows-/Linux-Zielvolumes und physische Persistenz; frische Rechner und
OS-Mindestversionen; vollständige SDK-/Systemruntime-Zuordnung.

Nächster Textschritt: gemeinsame Glyphengeometrie auch für Editor, Suche,
Formulare/einfache Labels, Cursor, Auswahl, IME und native Provider verwenden.
Neue Reader-CI 37751197759 prüfen; Intel-Mac-Paketabnahme besteht.

Eigener Code MIT. Signaturkonten fehlen; vertraulicher Sicherheitskanal ist
angefragt. Vollständiger Auftrag bleibt aktiv. **1.0 erst nach ausdrücklicher
Freigabe**; abschließende Produkttext-Bereinigung erst nach Vollständigkeit.
