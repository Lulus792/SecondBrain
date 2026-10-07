# SecondBrain: aktueller Stand

Stand: 7. Oktober 2026. Maßgeblich sind [STATUS](../../docs/STATUS.md),
[Plattformnachweise](../../docs/PLATTFORMEN.md) und [Release-Aufgaben](../../docs/RELEASE.md).
Frühere Arbeit und Rückfallkopien stehen im [Journal](journal/2026-10-07.md).

## Produkt und geprüfte App

Eigene C17-Desktop-App für Mensch und KI: Projekte/Notizen verwalten, lesen,
bearbeiten, suchen, archivieren und sichern; Quellen lesen und gespeicherten
KI-Kontext kopieren. Lumen-Sternkarte, eigene Glaskarten/Icons, direkte Pfeil-
navigation, Raumfahrt, weiches Scrollen und große Leseansicht sind implementiert.
Entwürfe, Originalbytes und erkannte Konflikte bleiben geschützt.

Dist enthält lokal geprüftes **0.9.18, Build ee71cc4c74aa**, Intel/macOS Release.
44/44 Tests (287,66 s), gezielte ASan/UBSan mit 128 Assertions ohne Diagnose und
entpacktes Paket bestehen. Desktop, Tastatur, Sicherung, Neustart und CLI sind
geprüft; eigene Instanz in der installierten App geladen und Raster betrachtet.
Die bisherige App 0.9.16 bleibt als Rückfallkopie in build/previous-dist-0.9.16-20261007-212254.

Quellschritt ee71cc4 ist normal nach origin gepusht.
[Neue CI 37673520042](https://github.com/Lulus792/SecondBrain/actions/runs/37673520042)
ist bei der letzten Abfrage noch in der Warteschlange. Daraus keine neue Windows-,
Linux- oder ARM64-Abnahme ableiten. Frühere Plattformnachweise bleiben historisch.

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

Nächster Schritt: neue CI-Nachweise übernehmen und weitere Release-Arbeiten. Die C-Graphfunktion erhält den
letzten gültigen Graph. Die Desktop-Sicht benötigt dafür noch stabile Inventur-
kennungen; alte Indizes dürfen keine neue Notizliste beschriften.

## Weitere offene Release-Arbeit

Bidi, visuelle/native Textgeometrie, IME, native Tabellenmatrix, transitive
Lizenzprüfung und übrige Release-Aufgaben bleiben offen. Support/Beitragsregeln
und Abhängigkeitswartung sind veröffentlicht; vertraulicher Sicherheitskanal
ist angefragt. Menschliche VoiceOver/NVDA/Orca-, Dialog-, Geräte-/Langzeit- und
weitere Vollvolume-Abnahmen fehlen. CI/Raster ersetzen sie nicht.

Der vollständige Auftrag bleibt aktiv. Eigener Code: MIT; Signaturkonten fehlen.
1.0 erst nach ausdrücklicher Freigabe, abschließende Produkttext-Bereinigung
nach den festgelegten Voraussetzungen. Verträge und Originale in [SOURCES](SOURCES.md).
