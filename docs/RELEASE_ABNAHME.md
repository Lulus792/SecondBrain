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
| Sichtbare Einstellungsfehler beim Beenden, Wiederholung und Rückkehr | [Fehlerkarte](EINSTELLUNGEN_BEENDEN.md); lokale, Sanitizer- und native Windows-/Linux-Prüfungen |
| Normaler sichtbarer Start und Accessibility vor dem ersten Anzeigen | [Startdiagnose](LAUFZEIT_START.md); Windows-/Linux-Release-Pakete und Intel-Mac geprüft |
| Lesbare Oberfläche bei Materialfehlern | [Materialrückfall](MATERIAL_RUECKFALL.md), Software-/Metal- und gezielte Speicherfehlerprüfungen |

„Implementiert“ bedeutet hier Code mit den genannten begrenzten Prüfungen.
Die vollständige menschliche und zugesagte Plattformabnahme folgt daraus nicht.
Schrift-/Markdownumfang ist ausdrücklich begrenzt und dokumentiert; zukünftige
Schemaänderungen sind kein bereits vorhandener Migrationsmechanismus.

## Aktueller ausgeführter Stand

Installiert: **0.9.74 / eabf5b3ff114**, Intel/macOS 14.6.1; 98 Dateien paketgleich.
App-SHA256: `d0b039034dce04ca8a5cf712bdc7858810040008aeb1715acb2dfc710fc65ff7`.
Die [Sternbeschriftungen](STERNBESCHRIFTUNGEN.md) sind im Acht-Test-Lauf gezielt
geprüft. Saubere Versions-/Startprüfung, entpacktes Paket und 73→74→73-Wechsel
bestehen; 22 Gedächtnisdateien beim installierten Snapshot und normalen Metal-
Start unverändert. CI38197916019 besteht unter Windows/Linux in Debug/Release
mit jeweils90 Tests und beiden Release-Paketen; Mac queued. Heruntergeladene
Pakete/Nachweise SHA-geprüft, App-Hashes stimmen mit den nativen Berichten überein.
Der folgende vollständige 89er-/native Plattformstand gehört zur Vorversion.

Vorige installierte Fassung: **0.9.73 / 15e63b4f7960**, sauberer Intel/macOS-Release auf
macOS 14.6.1 / 23G93. 98 Dateien paketgleich. App-SHA256:
`72fb1cd672adc6b5bc8b8cc6601de1bb8d35e6e5aefeb01964cc4034a17d76b6`.
Die korrigierte Startreihenfolge besteht lokal **89/89 Tests zu e66e023**
(517,45 s), Binärhash und 211 Produktinputs unverändert. Danach ändern sich nur
Prüfer, CI-Nachweissicherung und Dokumentation: 211 Produktinputs bleiben gleich.
Die saubere 15e63b4-Fassung bestätigt beide Versionswege und normalen Start in
1,22 s. Ihre Buildkennung unterscheidet die Binärdateien; der vollständige Lauf
gehört ausdrücklich zu e66e023, Paket und Installation zu 15e63b4.

Das entpackte Paket besteht 126 Desktop-, 167 Tastatur- und 75 Sicherungsbedingungen,
Einstellungen in zwei Prozessen, CLI, Lizenzen, Versions- und Laufzeitprüfung.
Normaler Paketstart: Metal/Cocoa, sichtbares Fenster, vier Frames, 394 Module,
13 isolierte Projektdateien erhalten. Echter **72→73→72-Wechsel** erhält 20
Testdateien und Einstellungen; Sicherung/Wiederherstellung bestehen.
Drei Updateaufnahmen und die installierte Ansicht betrachtet. Beim Software-
Snapshot und dem zusätzlich geprüften normalen installierten Start bleiben alle
21 Gedächtnisdateien unverändert. App: minos 14.0, SDK 15.2; kein Nachweis auf
macOS 14.0/ARM64. [Startfehler und genaue Abdeckung](LAUFZEIT_START.md).

[CI38195529730](https://github.com/Lulus792/SecondBrain/actions/runs/38195529730)
zu 15e63b4: Windows/Linux Debug/Release vollständig erfolgreich, einschließlich
beider Release-Pakete. SHA256-verifizierte native Paketberichte bestätigen
Direct3D11/Windows mit 52 Modulen beziehungsweise OpenGL/X11 mit 65 Modulen;
jeweils sichtbares Fenster, vier Frames und unveränderte Testdaten.
Mac-CI wartet auf Runner. Server 2025/Xvfb ersetzen keine Windows-11-/Wayland-
oder menschliche Screenreaderabnahme. Originale unter
`build/runtime73-fixed-verification/`, `build/runtime73-verification/`
und `build/runtime73-update/`.

## Projektgedächtnis nach der Prüfung

Aktueller Stand, Entscheidungen und Quellen sind auf den benötigten Kern gekürzt.
Die drei vollständigen früheren Fassungen liegen im Projektarchiv; relative Links
wurden reversibel an den Archivordner angepasst. Alle 22 Entscheidungskennungen
bleiben vorhanden. Der kopierbare Kontext sinkt von 46.118 auf 18.012 Bytes
(rund 61 %). 523 lokale Verweise geprüft; elf ältere Journalverweise korrigiert.
21 Gedächtnisdateien beim Öffnen in der installierten App unverändert,
aktuelle Darstellung angesehen. Das ist Pflege der eigenen Instanz, keine
automatische Änderung anderer Second Brains. Dieser historische Pflegeabschnitt gehörte zur damaligen 0.9.68; aktuelle Installation oben.

## Noch nötige Abnahmen und Entscheidungen

| Offener Punkt | Benötigter Abschluss |
| --- | --- |
| Betriebssystemzusagen | [Vorschlag](PLATTFORM_VORSCHLAG.md) bestätigen und tatsächlich abnehmen: macOS 14 Intel/ARM64, Windows 11 25H2 x64, Ubuntu 24.04 LTS x64; zusätzlich aktuelle Systeme |
| Frische Nutzerrechner | Entpacktes aktuelles Paket starten und gesamten Projekt-/Notiz-/Sicherungs-/Updateablauf bedienen; CI-Konfiguration genügt nicht |
| Menschliche Bedienung/Barrierefreiheit | VoiceOver, NVDA, Orca, native IME und Dialoge; Kontraste/Custom-Paletten, große Schrift, reale Systemvorgaben und RTL/Textauswahl |
| Alltag und Leistung | Reale größere Wissensbasen, schwächere Geräte, Trackpads, lange Sitzungen, verschiedene GPU-Treiber, Monitore und tatsächliche Pixeldichtewechsel |
| Dateien und Persistenz | Weitere Dateisysteme und physische Persistenz; [native volle Backup-Volumes](VOLLER_SPEICHER.md) sind bestätigt; fünf [Schreib-/Modellfälle](VOLLER_SPEICHER.md) sind auf HFS+/NTFS/ext4 bestätigt; weitere Dateisysteme und physische Persistenz bleiben offen |
| Vollständige mitgelieferte Laufzeit-/Lizenzzuordnung | Tatsächliche Binär-/Toolchainanteile und dynamische/transitive Komponenten final abgleichen; [Inventur](LIZENZ_INVENTUR.md), [Rust](RUST_RUNTIME_NACHWEIS.md), [Laufzeit](PAKET_LAUFZEIT.md), [Linkerbestand](LINKER_BESTAND.md) |
| Vertraulicher Sicherheitskanal | GitHub Private Vulnerability Reporting gewählt; Aktivierung nach GitHub-Anmeldung und tatsächliche Erreichbarkeit noch offen; [Sicherheitsrichtlinie](../SECURITY.md) |
| Signierung und Distribution | Apple-Developer-Konto/Windows-Zertifikat fehlen; tatsächliche Signierung/Notarisierung und dauerhafte aktuelle Pakete vor Freigabe prüfen |
| Fertige Texte und 1.0 | Redaktionelle Schlussprüfung erst beim vollständigen Produkt, danach ausdrückliche Nutzerfreigabe; keine automatische 1.0-Setzung |

## Nächster Schritt

Die [Windows-Kontrastfarben](SYSTEM_KONTRASTFARBEN.md) sind ab 0.9.69 im Adapter
und UI-Code implementiert und lokal mit synthetischen Paletten nachgeprüft.
SDK-/Lebensdauer- und neue Windows-Release-Paketprüfung bestehen nativ.
Benutzerdefinierte Systemthemen und menschliche Kontrastbedienung bleiben offen;
synthetische Farben auf dem Mac ersetzen diese nicht. Weitere technische
Release-Abnahmen anhand der Tabelle fortführen.
Den erfolgreichen Gesamt-/CI-Stand im Projektgedächtnis kurz halten. Offene technische
Abnahmen bleiben Arbeitsumfang; erforderliche Geräte/Konten und Nutzerentscheidungen
werden nicht durch weitere kleine Codeänderungen oder Statusnotizen ersetzt.

## Historischer begrenzter Prüfstand: 0.9.69

Der frühere 84er-Gesamtlauf und vier bestätigte Plattformjobs gehörten
zur 0.9.68; aktuelle Ergebnisse stehen oben. Inzwischen ist [0.9.69 mit Kontrastfarben](SYSTEM_KONTRASTFARBEN.md)
lokal gezielt geprüft und sauber installiert, einschließlich Paket und echtem
68→69→68-Wechsel. Die Windows-Debug-/Release- und Linux-Releasejobs einschließlich
Release-Paketen bestehen. Linux Debug scheitert in der Textvorbereitung; die
Palettenprüfung besteht auch dort. Mac-CI bleibt queued, benutzerdefinierte Themen
und menschliche Prüfung offen. Kein neuer lokaler 85er-Gesamtlauf zu 0.9.69.

0.9.70 korrigiert die Schemanamenfreigabe, 0.9.71 einen reproduzierten
[FreeType-Listenfehler](SCHRIFT_LEBENSDAUER.md). Acht lokale gezielte Tests sowie
positive/negative ASan-/UBSan-Vergleiche sind dokumentiert. Der neue Gesamt-, native CI-, Paket- und Installationsstand steht oben; die damalige Installation
war 0.9.71. Weitere dort benannte Abnahmen bleiben offen.
