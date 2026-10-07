# SecondBrain: aktueller Stand

Stand: 8. Oktober 2026. Maßgeblich sind [STATUS](../../docs/STATUS.md),
[Plattformnachweise](../../docs/PLATTFORMEN.md) und [Release-Aufgaben](../../docs/RELEASE.md).
Die [Übergabe vom 8. Oktober](journal/2026-10-08.md) hält die Fortsetzung fest.
Frühere Arbeit und Rückfallkopien stehen im [Journal vom 7. Oktober](journal/2026-10-07.md).

## Produkt und geprüfte App

Eigene C17-Desktop-App für Mensch und KI: Projekte/Notizen verwalten, lesen,
bearbeiten, suchen, archivieren und sichern; Quellen lesen und gespeicherten
KI-Kontext kopieren. Lumen-Sternkarte, eigene Glaskarten/Icons, direkte Pfeil-
navigation, Raumfahrt, weiches Scrollen und große Leseansicht sind implementiert.
Entwürfe, Originalbytes und erkannte Konflikte bleiben geschützt.

Dist enthält lokal geprüftes **0.9.21, Build d7e328d2afa2**, Intel/macOS Release.
Die letzten drei UI-/Native-/Tastaturprüfungen bestehen. Entpacktes Paket besteht
mit 126 Desktop-, 150 Tastatur- und 75 Sicherungsassertions, zwei Neustart-
prozessen und produktiver CLI-Sicherung/Wiederherstellung. Beide Lizenzsammlungen
stimmen in Paketwurzel/App bytegenau mit dem Manifest überein. Eigenes Gedächtnis
in installierter App geladen und Raster betrachtet; seine Dateien unverändert.
0.9.19 bleibt als Rückfallkopie in build/previous-dist-0.9.19-20261007-233745.

Quellschritt 9ab187a ist normal nach origin gepusht.
[Neue CI 37680520037](https://github.com/Lulus792/SecondBrain/actions/runs/37680520037)
besteht in beiden macOS-Architekturen (Debug/Release) und Windows-Debug.
Windows-Release scheitert beim UI-Teststart, ohne öffentlich lesbares Testlog;
beide Linux-Jobs sind abgebrochen. Ursache bleibt ungeklärt, Diagnose ist angefragt.
[Neue CI 37673520042](https://github.com/Lulus792/SecondBrain/actions/runs/37673520042)
besteht inzwischen in allen acht Desktop-Debug/Release-Jobs auf Windows, Linux
und macOS ARM64/Intel einschließlich entpackter Pakete. Das ist die Abnahme
zu 0.9.18; neue 0.9.19-Nachweise folgen.

## Laufende Interaktionspolitur

0.9.17 bindet den eigenen [Dokumentbaum](../../docs/DOKUMENTBAUM.md) an
Titel/Reader/Tabellen/Graph; Quell-/Datenprüfungen und lokale UI-Abnahme bestehen.
Source 08b0911 ist gepusht; die Integration gehört zur neuen geprüften App.

Neues Nutzerfeedback mit Screenshot priorisiert ab 0.9.18
[Reaktionszeit und Inhaltsflächen](../../docs/INTERAKTION.md): frühe Fahrt, kurze
Kartenüberblendung, genauer Scroll-/Zeigerpfad, passende Dialogmaße, gut lesbare
Hinweise/Kürzel und Suchfeld. Overlay-Endstand besteht in 44 Tests; danach
gefundene Reader-/UI-Pufferfehler sind korrigiert. Gezielte ASan/UBSan besteht
mit 128 Assertions. Endgültig 44/44 Tests in 287,66 s bestanden. Der Materialvergleich besteht in 48 exakt gleichen Rastern.
Keine pauschale FPS- oder menschliche assistive Abnahme.

Nächster Schritt: Windows-Teststarterdiagnose und neue CI-Nachweise auswerten,
Rust-/Systemruntime-Inventur und weitere Release-Arbeiten fortsetzen. Die C-Graphfunktion erhält den
letzten gültigen Graph. Die Desktop-Sicht
hält ab 0.9.19 eine eigene zusammenhängende Inventur mit stabilen Kennungen.
Gezielte Fehler-/Wiederherstellungsprüfung besteht. Neues Feedback führt zu
[Textlayoutcaches, Startfokus, Suchende, Caret und Anlegekarten](../../docs/NAVIGATION_POLITUR.md).
48 vollständige Rasterprüfsummen/Höhen stimmen mit ungecachetem Reflow überein.
46/46 Tests (424,71 s), letzte Nachprüfung 4/4 (155,29 s), gezielte ASan/UBSan
mit 95 Navigations- und 34 Graphassertions ohne Diagnose bestanden. Entpacktes Intel/macOS-Paket und Installation
bestehen mit 126 Desktop-, 143 Tastatur- und 75 Sicherungsassertions sowie Neustart/CLI.

## Weitere offene Release-Arbeit

Bidi, visuelle/native Textgeometrie, IME, native Tabellenmatrix, transitive
Lizenzprüfung und übrige Release-Aufgaben bleiben offen. Support/Beitragsregeln
und Abhängigkeitswartung sind veröffentlicht; vertraulicher Sicherheitskanal
ist angefragt. Menschliche VoiceOver/NVDA/Orca-, Dialog-, Geräte-/Langzeit- und
weitere Vollvolume-Abnahmen fehlen. CI/Raster ersetzen sie nicht.

Der vollständige Auftrag bleibt aktiv. Eigener Code: MIT; Signaturkonten fehlen.
1.0 erst nach ausdrücklicher Freigabe, abschließende Produkttext-Bereinigung
nach den festgelegten Voraussetzungen. Verträge und Originale in [SOURCES](SOURCES.md).

0.9.20 erweitert die Lizenzansicht mit 113 zugeordneten Cargo-Komponenten und
SDL-/HarfBuzz-Quellenhinweisen. 47/47 CTests und nachfolgende
gezielte Prüfungen bestehen; beide großen Sammlungen sind per Tastatur gelesen/
kopiert. 20 Python-Prüfungen bestehen. Paketabnahme ist in 0.9.21 abgeschlossen. Rust-/Systemruntime-Abgleich
und vollständige transitive Release-Abnahme bleiben offen.

0.9.21 überspringt unsichtbare Literal-Zeichenarbeit bei unverändertem Layout/
nativer Semantik. Drei Nachprüfungen und 150 Tastaturassertions bestehen;
drei vollständige Raster sind bytegleich. Lokale Layoutzeit bei 600 gleichen
Testframes 81,55 → 4,88 s. Saubere Paketierung und Installation bestehen.

[Quell-CI zu 1f1e23c](https://github.com/Lulus792/SecondBrain/actions/runs/37690493714)
ist beim Abschluss queued; neuer Gesamt-Plattformnachweis steht aus.

CI-Einstieg normalisiert native CMake-Pfade; Python meldet auch fehlende Logs
und erhält frühen Fehlerkontext. 7 Prozessfälle, 23 Python-Unittests und
31/31 lokale Release-Kernprüfungen bestehen. Neue Windows-Abnahme folgt.

Rust-UI-Provenienz ist als CI-Aufzeichnung umgesetzt und mit echtem Cargo
geprüft; 25 Python-Prüfungen bestehen. macOS-Archive zeigen Compilerpfad
48a229c/1.98.1, lokale Cargo-Probe 1.99.0/b940084d7eb6. Library-Lock des
Mac-Standes enthält 30 Registry-Pakete; zusätzliche Original-Lizenzen und
tatsächliche Windows-/Linux-Compilerzuordnung bleiben offen.

Windows-Teststarterursache aus CI 37691907747 belegt: kurzer TEMP-Pfad
RUNNER~1 wird mit korrekt aufgelöstem runneradmin-Pfad als Text verglichen.
Fixture vergleicht jetzt kanonische Pfade; reale Aliasprobe und insgesamt
26 Python-Tests bestehen lokal. Windows-Nachprüfung folgt. GitHub-API aktuell
rate-limited; das ist keine Aussage über den terminalen Status laufender Jobs.

CI edfa86b bestätigt Windows-Debug; Release findet eine Marker-Lesesperre
in der Abbruchprobe. Begrenzte Synchronisation korrigiert und lokal mit
3 Prozessfällen, 6 Produktionsabbrüchen und 29 Python-Tests geprüft.
Neue Windows-/Gesamtabnahme folgt; Rust-Sammlung ist noch in Arbeit.

CI 82c2970 besteht in allen 20 Jobs, acht Desktop-Varianten und vier Paketen.
Windows-/Linux-Cargo bestätigen 1.98.1/48a229c. 0.9.22 ergänzt Rust-Originale
für zwei geprüfte Quellenstände; 36 Python-Prüfungen, Ressourcen und 157
Tastaturassertions bestehen. Paket-/neue Plattformabnahme folgen vor Installation.
