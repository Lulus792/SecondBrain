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

Installiert: **0.9.33, Build d48ee9bf7075**, Intel/macOS Release.
Entpacktes Paket besteht mit 126/157/75 Assertions, zwei Prozessneustarts,
Produktions-CLI und Runtime-Importprüfung. Lizenzbündel exakt, eigenes Gedächtnis
beim Laden unverändert, installierte Ansicht betrachtet. Rückfallkopie:
`/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.28-20261008-082929`.

## Geprüfte Fortschritte

- Neue C-Bidi-Grundlage im UI-Prüfbuild: 861.948 Unicode-18-Richtungsfälle,
  438 Spiegelpaare und alle Unicode-Positionen bestehen; ASan/UBSan im
  dokumentierten Teilscope und bytegenaue Neugenerierung ebenfalls.
  32/32 Kernprüfungen ohne UI bestehen. Noch nicht an die sichtbare App
  angeschlossen; [Vertrag/Nachweise](../../docs/BIDI.md).
- Erste neue CI bestätigt Linux Debug/Release sowie Mac ARM Debug/Release
  und Intel Release. Windows scheitert noch beim Checkout-abhängigen C-Tabellen-
  Hash. Explizite LF-/Originalbyte-Regeln und zwölf Git-Checkoutfilter-Hashes
  mit Windows-autocrlf sind geprüft; native Nachprüfung folgt.
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

## Nächste Arbeiten

0.9.33 ist vollständig im dokumentierten automatisierten Umfang abgenommen.
Weitere Release-Arbeiten: Bidi/visuelle/native Textgeometrie und reale Eingabemethoden;
menschliche VoiceOver/NVDA/Orca-, Dialog-, Geräte-, Mehrmonitor-/Langzeitabnahme;
volle Windows-/Linux-Zielvolumes und physische Persistenz; frische Rechner und
OS-Mindestversionen; vollständige SDK-/Systemruntime-Zuordnung.

Nächster Textschritt: geprüften Absatzplan vor Schrift-/Stilaufteilung und
Umbruch anbinden, anschließend endgültige Glyphengeometrie für Cursor,
Auswahl, IME und native Provider gemeinsam verwenden. Neue CI-Abnahme prüfen.

Eigener Code MIT. Signaturkonten fehlen; vertraulicher Sicherheitskanal ist
angefragt. Vollständiger Auftrag bleibt aktiv. **1.0 erst nach ausdrücklicher
Freigabe**; abschließende Produkttext-Bereinigung erst nach Vollständigkeit.
