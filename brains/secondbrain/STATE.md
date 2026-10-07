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

[Dokumentbaum](../../docs/DOKUMENTBAUM.md) in eigenem C: 307 Originalfälle und
683.008 Assertions mit gezielter ASan/UBSan geprüft. Ab Arbeitsfassung 0.9.17
verwenden Titel, Reader, Tabellen und Graph dieselben Projektionen/Referenzen.
[UI](../../docs/CONTAINER_UI.md) ergänzt Listen, Nummern, Zitatlinie und native
Container. Originalpositionen, Dateien und Entwürfe bleiben erhalten; gültige
Dateien bleiben bei Parsergrenzen per Dateiname/Literalmodus erreichbar.

Erster Release-Lauf 43/43 in 298,53 s; abschließend 29/29 Kern-Debug in 29,08 s.
Abschließender Release-Lauf: 43/43 in 271,45 s. Eigene native ASan/UBSan: 552 Assertions; Paketabnahme läuft noch. Dist bleibt geprüftes 0.9.16.
Nächster Schritt: Abschlussnachweise und neues Paket, danach weitere Markdown-/
Textregeln und native Plattformabnahme. a45d7e2 ist jetzt gepusht;
[CI 37656257074](https://github.com/Lulus792/SecondBrain/actions/runs/37656257074)
besteht in allen zwölf Windows-/Linux-Jobs. Acht macOS-Jobs warten noch.
Der frühere Windows-Debug-Ausfall tritt dort nicht auf; Ursache ungeklärt.
Die neue App-Anbindung braucht ihre eigene Plattformabnahme.

## Weitere offene Release-Arbeit

Bidi, visuelle/native Textgeometrie, IME, native Tabellenmatrix, transitive
Lizenzprüfung und übrige Release-Aufgaben bleiben offen. Support/Beitragsregeln
und Abhängigkeitswartung sind veröffentlicht; vertraulicher Sicherheitskanal
ist angefragt. Menschliche VoiceOver/NVDA/Orca-, Dialog-, Geräte-/Langzeit- und
weitere Vollvolume-Abnahmen fehlen. CI/Raster ersetzen sie nicht.

Der vollständige Auftrag bleibt aktiv. Eigener Code: MIT; Signaturkonten fehlen.
1.0 erst nach ausdrücklicher Freigabe, abschließende Produkttext-Bereinigung
nach den festgelegten Voraussetzungen. Verträge und Originale in [SOURCES](SOURCES.md).
