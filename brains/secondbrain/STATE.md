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

Dist enthält lokal geprüftes 0.9.16, Build b19b47624512. Das Intel/macOS-Paket
besteht einschließlich Desktop/Tastatur/Sicherung/Neustart/CLI; eigenes Gedächtnis
und Raster betrachtet. Gemeinsame Referenzen, Unicode-Namen und Scroll-/Fokus-
abstand sind integriert. Letzte lokale Release-Abnahme: 40 Tests; eigene
Kern-/native Sanitizer bestehen im dokumentierten Umfang. UI-Bericht SBUI-052/053.

[Lauf 37646348381](https://github.com/Lulus792/SecondBrain/actions/runs/37646348381)
zu b19b476 besteht in allen zwölf Windows-/Linux-Jobs, einschließlich Desktop
Debug/Release und tatsächlich entpackter Pakete. Neue macOS-CI noch in der
Warteschlange; daraus keine ARM64-Gesamtabnahme ableiten. Die vorher ausstehenden
Commits sind normal nach GitHub gepusht; keine Historie wurde umgeschrieben.

## Laufende Interaktionspolitur

0.9.17 bindet den eigenen [Dokumentbaum](../../docs/DOKUMENTBAUM.md) an
Titel/Reader/Tabellen/Graph; Quell-/Datenprüfungen und lokale UI-Abnahme bestehen.
Source 08b0911 ist gepusht; Dist bleibt 0.9.16 bis zur neuen Paketabnahme.

Neues Nutzerfeedback mit Screenshot priorisiert ab 0.9.18
[Reaktionszeit und Inhaltsflächen](../../docs/INTERAKTION.md): frühe Fahrt, kurze
Kartenüberblendung, genauer Scroll-/Zeigerpfad, passende Dialogmaße, gut lesbare
Hinweise/Kürzel und Suchfeld. Overlay-Endstand besteht in 44 Tests; danach
gefundene Reader-/UI-Pufferfehler sind korrigiert. Gezielte ASan/UBSan besteht
mit 128 Assertions. Endgültig 44/44 Tests in 287,66 s bestanden. Der Materialvergleich besteht in 48 exakt gleichen Rastern.
Keine pauschale FPS- oder menschliche assistive Abnahme.

Nächster Schritt: Commit/Push, Paketabnahme und neue App
installieren; danach weitere Release-Arbeiten. Die C-Graphfunktion erhält den
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
