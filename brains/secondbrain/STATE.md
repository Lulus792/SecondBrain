# SecondBrain: aktueller Stand

Stand: 8. Oktober 2026. Originale: [STATUS](../../docs/STATUS.md),
[Plattformnachweise](../../docs/PLATTFORMEN.md), [Release-Aufgaben](../../docs/RELEASE.md).
Die [Übergabe vom 8. Oktober](journal/2026-10-08.md) hält die Fortsetzung fest;
frühere Prüfungen und Rückfallkopien stehen im [Journal vom 7. Oktober](journal/2026-10-07.md).

## Produkt und installierte App

Eigene C17-Desktop-App für Mensch und KI: Projekte/Notizen verwalten, lesen,
bearbeiten, suchen, archivieren und sichern; Quellen und gespeicherten Kontext
kopieren. Lumen-Sternkarte, Glaskarten/Icons, direkte Pfeilnavigation, Kamerafahrt,
weiches Scrollen, Startfokus, Textcursor und große Leseansicht sind implementiert.
Entwürfe, Originalbytes und erkannte Konflikte bleiben geschützt.

Dist enthält **0.9.28, Build 7f5b63432062**, Intel/macOS Release. Entpacktes Paket
besteht mit 126 Desktop-, 157 Tastatur-, 75 Sicherungsassertions, zwei Neustarts
und CLI. Alle drei Lizenzsammlungen stimmen bytegenau mit ihren Manifesten
überein. Eigenes Gedächtnis geladen/Raster betrachtet, Dateien unverändert.
Rückfallkopie: build/previous-dist-0.9.26-20261008-071024.

[CI 37704298271](https://github.com/Lulus792/SecondBrain/actions/runs/37704298271)
zu 7f9acb8 besteht in allen 20 Jobs einschließlich acht Desktop-Varianten und
vier entpackter Release-Pakete. Frühere Windows-TEMP- und Kontrollpunkt-
Synchronisationsfehler sind im tatsächlichen Umfang nachgeprüft.

## Laufende Release-Arbeit

[Runtime-Importprüfung](../../docs/PAKET_LAUFZEIT.md) ist am entpackten Paket
angebunden. Echtes Shared-Library-Fixture erkennt eine entfernte Bibliothek;
0.9.23: alle vier Release-Paketprüfungen bestehen. 39 Python-Fälle bestehen mit
explizitem Skip der lokalen Windows-Kategorieprobe. Native Windows-CI bestätigt
die statische MSVC-CRT der UI-DLL; Scanner stoppt an der OS-Systemgrenze und
behält separate Redistributables sichtbar. Keine frische Nutzerrechner-/Mindestversionsabnahme
allein aus Systempfaden, CI oder Binärmetadaten ableiten.

[Rust-Quelleninventur](../../docs/RUST_RUNTIME_NACHWEIS.md) umfasst geprüfte
Standardbibliotheksstände 1.98.1/1.99.0 und 20 zugeordnete Registry-Versionen.
Windows/Linux-Cargo zeigen in der ausgeführten CI 1.98.1/48a229c. Vollständige
Zuordnung tatsächlich gelinkter SDK-/Systemanteile und abweichender Toolchains
bleibt offen; zusammengesetzte Lizenzbedingungen sind erhalten.

0.9.24 ergänzt den nativen Cursor-Anker und Popup-/Elternfokus. Lokaler
Release-Neubau besteht mit 49/49; letzte Popup-Nachprüfung mit 3/3
und 134 Editorassertions besteht. Lokales Paket ist abgenommen/installiert;
[CI 37717307982](https://github.com/Lulus792/SecondBrain/actions/runs/37717307982)
zu 1c2f323 besteht in allen 20 Jobs inklusive acht Desktop-Varianten
und vier entpackter Release-Pakete.

0.9.25 implementiert vorläufige IME-Komposition, geordneten Feldwechsel und
Fokusbestand bei Layoutänderungen. 51/51 lokale Release-Prüfungen und gezielte
ASan/UBSan bestehen. Ein provozierter 16-Bit-Textbreitenüberlauf ist durch
Float-Geometrie und sichtbare Textausschnitte behoben. Details: [IME](../../docs/IME.md).

[CI 37723366327](https://github.com/Lulus792/SecondBrain/actions/runs/37723366327)
zu 90bdb78 besteht in allen 20 Jobs inklusive vier entpackter Pakete.
Lokales Paket ist vollständig abgenommen und installiert.

0.9.26 erweitert Feldbindung auf normale Texte, erhält Fokus-/Caret-/Formular-
reihenfolge und verarbeitet druckbare Zeichenfolgen zusammen. 51 lokale
Prüfungen bestehen am selben Quellstand (16 vor, 35 nach Platzbereinigung);
gezielte ASan/UBSan bestehen mit 36/7/119 Assertions. Mausfokus und Layout
bei Schriftwechsel sind nachgeprüft. Native CI-/Paketabnahme folgen.

Quellstand 598af64 gepusht; [CI 37727294396](https://github.com/Lulus792/SecondBrain/actions/runs/37727294396)
besteht in allen 20 Jobs inklusive vier entpackter Pakete. Lokal installiert.

0.9.27 / 6496f7b: Mac-Tabellenmatrix lokal nativ geprüft und gepusht.
0.9.28: Nutzerfeedback zu GPU-Übergang, Startfokus, Suchende und Formularlinien
umgesetzt. Gesamtprüfung fand einen ersten instabilen Formularrahmen;
Anfangsmaße und Meldungszustand sind korrigiert; alle 51 abschließenden
Release-Prüfungen bestehen (293,35 Sekunden). Eigene UI-/Desktop-/Renderer-
Sanitizer bestehen mit 135/124/36 Assertions. Paket vollständig geprüft und
installiert; [CI 37730572189](https://github.com/Lulus792/SecondBrain/actions/runs/37730572189)
läuft. Neue Plattformnachweise erst nach Abschluss.

Nächster Schritt: native Windows-Matrixabnahme für 0.9.29; anschließend
AT-SPI-Tabellenmatrix und weitere Release-Arbeiten.
Weitere Release-Arbeiten: Bidi/visuelle Textgeometrie/IME, native
Tabellenmatrix, Screenreader-/Dialog-/Geräte-/Langzeitabnahmen, volle Windows-/
Linux-Zielvolumes und physische Persistenz. Originalverträge stehen in SOURCES.
Support/Beitragsregeln und Wartungsablauf sind veröffentlicht; vertraulicher
Sicherheitskanal ist angefragt. Eigener Code MIT, Signaturkonten fehlen.

Der vollständige Auftrag bleibt aktiv. 1.0 erst nach ausdrücklicher Freigabe;
abschließende Produkttext-Bereinigung nach den festgelegten Voraussetzungen.

[CI 37730572189](https://github.com/Lulus792/SecondBrain/actions/runs/37730572189)
zu 7f5b634 besteht in allen 20 Jobs, ausdrücklich einschließlich vier
entpackter Pakete unter Windows x64, Linux x64 und macOS Intel/ARM64.
Menschliche Screenreader-/Geräteabnahme bleibt getrennt offen.

0.9.29 ergänzt Windows Grid/GridItem und Table/TableItem ohne neue Cargo-
Abhängigkeiten. Feste Quellvorbereitung mit Hashschutz ist geprüft; offizielle
Prüfung für Windows-Ziel und Lockfile besteht. Native C-Prüfung kontrolliert
Zellidentität, Header, Leerzellen, Indizes und schmale 200%-Ansicht.
Tatsächlicher Windows-Build/Ausführung folgen; installiert bleibt 0.9.28.

[CI 37732498154](https://github.com/Lulus792/SecondBrain/actions/runs/37732498154)
zu 23b0b76 besteht in allen 20 Jobs und vier entpackten Paketen. Die neue
Windows-Matrixprüfung besteht damit nativ in Debug und Release einschließlich
zwölf Zellen, Headerbeziehungen, ungültiger Indizes und 200%-Ansicht.
AT-SPI-Matrix und menschliche Tabellenbedienung bleiben offen.

0.9.30 ergänzt Table/TableCell unter Linux, direkte native Zellindizes mit
transparenten Row-Containern sowie übereinstimmende Eltern/Cache-/
Änderungswege. Linux-Zielcheck, Quellvorbereitung und vier Mac-Nachprüfungen
bestehen. Tatsächlicher Linux-Build und native Matrixprüfung folgen.

0.9.30-Linuxprüfung findet denselben GetRowColumnSpan-Signaturkonflikt in
Debug/Release. 0.9.31 folgt dem echten GNOME-C-Client/ATK-Server statt der
widersprüchlichen XML. Linux-Zielcheck (2,64 s) und zwei Mac-Nachprüfungen
(14,12 s) bestehen; neuer nativer Lauf folgt. Installiert bleibt 0.9.28.

0.9.32 korrigiert eine zweite übersehene schmale Linux-Baumerwartung auf
direkte Zellen. Die tatsächlichen Matrixabfragen bestehen bis dahin in
0.9.31-Release; zwei lokale Mac-Nachprüfungen bestehen. Neue CI folgt.

[CI 37736031017](https://github.com/Lulus792/SecondBrain/actions/runs/37736031017)
zu e479642 besteht in allen 20 Jobs und vier entpackten Paketen. Die echte
Linux-Matrix einschließlich Headern, Eltern, Indizes, Leerzellen und Spannen
besteht damit in Debug/Release bei 100/200 Prozent. Die zuvor gefundenen
Signatur-/Baumerwartungsfehler sind in diesem Umfang nachgeprüft. Menschliche
Screenreader-Bedienung und tatsächliche Textgeometrie bleiben offen.
