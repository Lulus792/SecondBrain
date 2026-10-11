# Aktuelle Abnahme vor 1.0

Stand: 12. Oktober 2026. Diese Übersicht ergänzt die ausführliche
[Release-Liste](RELEASE.md). Sie ersetzt keine offenen Anforderungen und setzt
weder Version 1.0 noch einen Release-Tag. Der Nutzer entscheidet über die Freigabe.

## Vorhandener Produktumfang

| Anforderung | Implementierter Stand und maßgebliche Belege |
| --- | --- |
| Eigene App für Mensch/KI in C; UI-Bibliotheken nur für Darstellung | [Architektur](ARCHITEKTUR.md), [Projektplan](PROJEKTPLAN.md), C17-Kern und UI-Ziele in CMake |
| Projektgedächtnisse anlegen, lesen, bearbeiten, suchen, archivieren; Quellen/Kontext/Sicherung | [README](../README.md), [Datenvertrag](DATENVERTRAG.md), [Sicherung](SICHERUNG.md) |
| Eigene wiederverwendbare Vorlage und eigenes Projektgedächtnis | [Vorlage](../templates/brain/START.md), [Projektinstanz](../brains/secondbrain/START.md); bestehende Instanzen werden nicht automatisch ersetzt |
| Lumen-Farben, Glaskarten, eigene Icons und Flächen mit passenden Innenabständen | [Gestaltung](UI_GALAXIE.md), [Interaktion](INTERAKTION.md), [Erstellaktionen](ERSTELLEN_ICONS.md), [Readeraktionen](READER_AKTIONEN.md) |
| Archiv verlassen und Bereiche weiter erreichen | [Archiv und Layout](UI_POLITUR.md), [Navigation](NAVIGATION_POLITUR.md), Navigation-/Tastaturprüfer |
| Sofortige Pfeilwahl, Startfokus, Kamerafahrt, groß lesen | [Tastaturvertrag](UI_TASTATUR.md), [Interaktion](INTERAKTION.md); Schutz ungespeicherter Änderungen bleibt |
| Suche mit eigenem Löschen-Icon, Suchende, Textcursor, Hinweise und Formulare | [Navigationspolitur](NAVIGATION_POLITUR.md), [Interaktion](INTERAKTION.md), [Wortnavigation](WORTNAVIGATION.md), [Hilfe](HILFE_RUECKKEHR.md) |
| Scrollposition und frühe Eingaben erhalten; Übergänge ohne alten Resize-Zuschnitt | [Scrollen beim Laden](SCROLLEN_BEIM_LADEN.md), [Übergangsbild](UEBERGANG_BILD.md) |
| Lesbare Oberfläche bei Materialfehlern | [Materialrückfall](MATERIAL_RUECKFALL.md), Software-/Metal- und gezielte Speicherfehlerprüfungen |

„Implementiert“ bedeutet hier Code mit den genannten begrenzten Prüfungen.
Die vollständige menschliche und zugesagte Plattformabnahme folgt daraus nicht.
Schrift-/Markdownumfang ist ausdrücklich begrenzt und dokumentiert; zukünftige
Schemaänderungen sind kein bereits vorhandener Migrationsmechanismus.

## Aktueller ausgeführter Stand

Installiert: **0.9.68 / 0b0fdacda4fa**, Intel/macOS 14.6.1/23G93, sauberer
Release-Build. Build- und installierte App sind bytegleich; SHA256
`ade288ab84195382a64f8c5b24fed8f1713b11aad0616c5a846b8ad958b8869e`.
Das Paket hat 98 geprüfte Dateien. Desktop-, Tastatur-, Sicherungs-, Versions-,
Laufzeit- und Updateprüfungen bestehen im dokumentierten Umfang. Der tatsächliche
67→68→67-Wechsel erhält 20 Testdateien und Einstellungen; 18 eigene Gedächtnisdateien
blieben beim damaligen Öffnen unverändert. [Nachweis](UEBERGANG_BILD.md).

Alle 84 lokalen Release-CTest-Prüfungen bestehen (707,14 s). Der Prozess ist
mit Exit 0 beendet; vor und nach dem Lauf bleiben Build-/installierte App
bytegleich. Originale: `build/release-audit-verification/full-ctest.log` und
`programs-before.json`. Das ist der aktuelle Gesamtnachweis, keine Übernahme einer
älteren Teilsuite. Package-/Update- und gezielte native Metal-/Sanitizerabnahmen
sind zusätzlich im oben verlinkten Übergangsvertrag dokumentiert.

Alle vier Windows-/Linux-Desktopjobs zu 0.9.68 / 0b0fdac bestehen tatsächlich,
einschließlich beider entpackter Release-Pakete:
[CI38181757771](https://github.com/Lulus792/SecondBrain/actions/runs/38181757771).
Linux-Release 114595624348, Linux-Debug 114595624494, Windows-Release 114595624587,
Windows-Debug 114595624614. Die Paketprüfschritte wurden einzeln auf `success`
geprüft. Native Mac-CI weiterhin queued; lokale Intel-Abnahme getrennt.
Windows Server 2025 und Ubuntu 24.04/Xvfb sind die CI-Umgebungen, keine tatsächliche
Windows-11-/Wayland-/menschliche Screenreaderabnahme.

## Projektgedächtnis nach der Prüfung

Aktueller Stand, Entscheidungen und Quellen sind auf den benötigten Kern gekürzt.
Die drei vollständigen früheren Fassungen liegen im Projektarchiv; relative Links
wurden reversibel an den Archivordner angepasst. Alle 22 Entscheidungskennungen
bleiben vorhanden. Der kopierbare Kontext sinkt von 46.118 auf 18.012 Bytes
(rund 61 %). 523 lokale Verweise geprüft; elf ältere Journalverweise korrigiert.
21 Gedächtnisdateien beim Öffnen in der installierten App unverändert,
aktuelle Darstellung angesehen. Das ist Pflege der eigenen Instanz, keine
automatische Änderung anderer Second Brains. App-Version bleibt 0.9.68.

## Noch nötige Abnahmen und Entscheidungen

| Offener Punkt | Benötigter Abschluss |
| --- | --- |
| Betriebssystemzusagen | [Vorschlag](PLATTFORM_VORSCHLAG.md) bestätigen und tatsächlich abnehmen: macOS 14 Intel/ARM64, Windows 11 25H2 x64, Ubuntu 24.04 LTS x64; zusätzlich aktuelle Systeme |
| Frische Nutzerrechner | Entpacktes aktuelles Paket starten und gesamten Projekt-/Notiz-/Sicherungs-/Updateablauf bedienen; CI-Konfiguration genügt nicht |
| Menschliche Bedienung/Barrierefreiheit | VoiceOver, NVDA, Orca, native IME und Dialoge; Kontraste/Custom-Paletten, große Schrift, reale Systemvorgaben und RTL/Textauswahl |
| Alltag und Leistung | Reale größere Wissensbasen, schwächere Geräte, Trackpads, lange Sitzungen, verschiedene GPU-Treiber, Monitore und tatsächliche Pixeldichtewechsel |
| Dateien und Persistenz | Volle Windows-/Linux-Zielvolumes, weitere Dateisysteme und physische Persistenz; vorhandene Crash-/ENOSPC-Prüfungen sind begrenzt |
| Vollständige mitgelieferte Laufzeit-/Lizenzzuordnung | Tatsächliche Binär-/Toolchainanteile und dynamische/transitive Komponenten final abgleichen; [Inventur](LIZENZ_INVENTUR.md), [Rust](RUST_RUNTIME_NACHWEIS.md), [Laufzeit](PAKET_LAUFZEIT.md) |
| Vertraulicher Sicherheitskanal | Nutzerentscheidung und tatsächliche Erreichbarkeit; öffentliche Issues sind kein vertraulicher Kanal |
| Signierung und Distribution | Apple-Developer-Konto/Windows-Zertifikat fehlen; tatsächliche Signierung/Notarisierung und dauerhafte aktuelle Pakete vor Freigabe prüfen |
| Fertige Texte und 1.0 | Redaktionelle Schlussprüfung erst beim vollständigen Produkt, danach ausdrückliche Nutzerfreigabe; keine automatische 1.0-Setzung |

## Nächster Schritt

Die [Windows-Kontrastfarben](SYSTEM_KONTRASTFARBEN.md) sind ab 0.9.69 im Adapter
und UI-Code implementiert und lokal mit synthetischen Paletten nachgeprüft.
Jetzt die tatsächliche SDK-/Paketabnahme und benutzerdefinierte Systemthemen
auf Windows abschließen; lokale Farben auf dem Mac ersetzen diese nicht.
Den erfolgreichen Gesamt-/CI-Stand im Projektgedächtnis kurz halten. Offene technische
Abnahmen bleiben Arbeitsumfang; erforderliche Geräte/Konten und Nutzerentscheidungen
werden nicht durch weitere kleine Codeänderungen oder Statusnotizen ersetzt.

## Neuerer begrenzter Prüfstand: 0.9.69

Die oben genannten 84 lokalen Tests und bestätigten vier Plattformjobs gehören
zur 0.9.68. Inzwischen ist [0.9.69 mit Kontrastfarben](SYSTEM_KONTRASTFARBEN.md)
lokal gezielt geprüft und sauber installiert, einschließlich Paket und echtem
68→69→68-Wechsel. Die Windows-Debug-/Release- und Linux-Releasejobs einschließlich
Release-Paketen bestehen. Linux Debug scheitert in der Textvorbereitung; die
Palettenprüfung besteht auch dort. Mac-CI bleibt queued, benutzerdefinierte Themen
und menschliche Prüfung offen. Kein neuer lokaler 85er-Gesamtlauf zu 0.9.69.

0.9.70 korrigiert die Schemanamenfreigabe, 0.9.71 einen reproduzierten
[FreeType-Listenfehler](SCHRIFT_LEBENSDAUER.md). Acht lokale gezielte Tests sowie
positive/negative ASan-/UBSan-Vergleiche sind dokumentiert. Neuer Gesamt-, nativer
CI-, Paket- und Installationsstand folgt separat; installiert bleibt 0.9.69.
