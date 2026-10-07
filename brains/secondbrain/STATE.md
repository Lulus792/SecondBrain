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

## Laufende Containerarbeit

[Quellprojektion](../../docs/CONTAINER_PLAN.md) in eigenem C bereitet die notwendige
Trennung von Originalquelle und aufbereitetem Containertext vor. Teilweise Tabs,
physische Spalten, Quellpositionen, Zeilenenden, Bereichsgrenzen und Rücknahme
fehlgeschlagener Anfügungen sind mit zuletzt 315.759 Assertions und gezielter ASan/UBSan
lokal geprüft. Der darauf aufbauende [Dokumentbaum](../../docs/DOKUMENTBAUM.md) ist jetzt
im Kern implementiert: 307 Originalfälle, 683.008 eigene Assertions und
gezielte ASan/UBSan bestehen; 29 Kern-Debug-Tests bestehen. Baum/Projektion
sind noch nicht mit der App verbunden; Listen-/Zitat-UI bleibt offen. Die installierte App bleibt 0.9.16.

Nächster Schritt: gemeinsame Titel-/Reader-/Tabellen-/Graphanbindung an den
Baum und seine Referenzen, danach sichtbare/nativ bedienbare Container
implementieren und gegen Originale prüfen. Neue
Plattformnachweise jeweils anhand tatsächlicher Jobs übernehmen.
Die Folge-CI zu 0a1e872 hat einen noch unbekannten Windows-Debug-Testfehler
ohne LastTest-Diagnose; Einstiegserfassung ist ergänzt, tatsächliche neue
Nachprüfung steht noch aus.

## Weitere offene Release-Arbeit

Bidi, visuelle/native Textgeometrie, IME, native Tabellenmatrix, transitive
Lizenzprüfung und übrige Release-Aufgaben bleiben offen. Support/Beitragsregeln
und Abhängigkeitswartung sind veröffentlicht; vertraulicher Sicherheitskanal
ist angefragt. Menschliche VoiceOver/NVDA/Orca-, Dialog-, Geräte-/Langzeit- und
weitere Vollvolume-Abnahmen fehlen. CI/Raster ersetzen sie nicht.

Der vollständige Auftrag bleibt aktiv. Eigener Code: MIT; Signaturkonten fehlen.
1.0 erst nach ausdrücklicher Freigabe, abschließende Produkttext-Bereinigung
nach den festgelegten Voraussetzungen. Verträge und Originale in [SOURCES](SOURCES.md).
