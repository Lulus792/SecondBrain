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

Dist enthält **0.9.24, Build 1c2f323b0fa5**, Intel/macOS Release. Entpacktes Paket
besteht mit 126 Desktop-, 157 Tastatur-, 75 Sicherungsassertions, zwei Neustarts
und CLI. Alle drei Lizenzsammlungen stimmen bytegenau mit ihren Manifesten
überein. Eigenes Gedächtnis geladen/Raster betrachtet, Dateien unverändert.
Rückfallkopie: build/previous-dist-0.9.23-20261008-042422.

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
zu 1c2f323 läuft. C17/Python und Linux-Release samt Paket bestehen;
übrige native Desktop-Abnahmen bleiben ausstehend.

Nächster Schritt: neue native Desktop-/Paketnachweise übernehmen.
Weitere Release-Arbeiten: Bidi/visuelle Textgeometrie/IME, native
Tabellenmatrix, Screenreader-/Dialog-/Geräte-/Langzeitabnahmen, volle Windows-/
Linux-Zielvolumes und physische Persistenz. Originalverträge stehen in SOURCES.
Support/Beitragsregeln und Wartungsablauf sind veröffentlicht; vertraulicher
Sicherheitskanal ist angefragt. Eigener Code MIT, Signaturkonten fehlen.

Der vollständige Auftrag bleibt aktiv. 1.0 erst nach ausdrücklicher Freigabe;
abschließende Produkttext-Bereinigung nach den festgelegten Voraussetzungen.
