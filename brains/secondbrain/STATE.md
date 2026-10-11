# SecondBrain: aktueller Stand

Stand: 12. Oktober 2026. Originale: [STATUS](../../docs/STATUS.md),
[Release-Liste](../../docs/RELEASE.md), [Quellen](SOURCES.md).
Historische Schritte und Messungen stehen in STATUS und im [Journal](journal/2026-10-11.md).

## Produkt und installierte App

Eigene C17-App für Mensch und KI: Projekte/Notizen anlegen, lesen, bearbeiten,
suchen, archivieren, sichern; Quellen lesen und Kontext kopieren. Lumen-Sternkarte,
Glaskarten, eigene Icons, direkte Pfeilnavigation, Kamerafahrt, Startfokus, Textcursor
und große Leseansicht sind implementiert. Originalbytes, Entwürfe und erkannte
Konflikte bleiben geschützt. Externe Bibliotheken nur UI; MIT.

Installiert **0.9.67/28cdea386abb**, sauberer Intel/macOS-Release-Build.
98 Dateien paketgleich;18 Gedächtnisdateien beim Öffnen unverändert.126 Desktop-,
167 Tastatur-,75 Sicherungsbedingungen, zwei Einstellungs-Neustarts, CLI-/Runtime-/
Lizenz-/Version bestehen. Tatsächliche App-Aufnahme angesehen. macOS14.6.1/23G93,x86_64.
Rückfall: `/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.66-20261012-003101`.

## Neuester geprüfter Fortschritt

0.9.66 berechnet bei Umbruchversuchen nur genaue kontextuelle Vorschubbreiten;
gewählte Zeilen behalten die vollständige Geometrie. Identische Batchaufträge
teilen einen Plan bei erhaltenen Eingabegrenzen. 83/83 lokale Release-Tests
bestehen (summiert 504,86 s), ASan/UBSan: 5385 Vorbereitungs-, 120 Reader- und
1019 Glyphenbedingungen. Externe UI nicht instrumentiert, Leaks aus, kein TSan.
Drei Metal-Paare: lange Notiz median 10,54 % früher vorbereitet, 6,64 % früher
übergangsfrei. Keine allgemeine Geräte-/Bediengarantie. Sauberes Paket geprüft und 98 Dateien paketgleich installiert; echter
65→66→65-Wechsel erhält 20 Dateien und Einstellungen. [Vertrag](../../docs/TEXT_VORBEREITUNG_OPTIMIERUNG.md).

0.9.67 erhält bei Materialfehlern die gewählte Darstellung und deckende Karten;
Teilpuffer sofort frei, erfolgreicher Versuch stellt Glas wieder her. 1122 Fehler-
und 352 normale Pixelbedingungen in Software/Metal, CPU-Parität 1012. ASan/UBSan
für beide Materialprüfer, 38 eigene C-Dateien / 60 Hashes, externe UI nicht
instrumentiert, Leaks aus. Fünf weitere UI-/Versionsprüfungen bestehen. Kein neuer
vollständiger lokaler 84er-Lauf. Sauberes Paket geprüft und installiert; echter
66→67→66-Wechsel erhält 20 Dateien/Einstellungen und Sicherung/Wiederherstellung.
[Vertrag](../../docs/MATERIAL_RUECKFALL.md). Übergangstextur-Pixelidentität bleibt offen.

## Aktueller Darstellungsschritt

0.9.64 unterscheidet kompakte Erstellaktionen durch Dokument-/Ordnerplusse.
Prioritäre Sternlabels werden zuerst reserviert, keine teilweise übermalten
normalen Wörter. Neun Darstellungen und18 SDL-Aktions-/Escapeabläufe geprüft;
9/9 passende Produktprüfungen bestehen (141,20s). Paket geprüft und installiert;
CI38171616070: alle vier Windows-/Linux-Desktopjobs einschließlich beider
Release-Pakete erfolgreich, native Mac-Jobs queued. [Vertrag](../../docs/ERSTELLEN_ICONS.md).
[Mindestversionsvorschlag](../../docs/PLATTFORM_VORSCHLAG.md): technische Voraussetzungen,
tatsächlich geprüfte CI-Umgebungen und offene Nutzerplattform-Abnahme getrennt.
Die Linux-App0.9.64 verlangt direkt GLIBC_2.39, die CLI GLIBC_2.34;
der Paketprüfer erfasst jetzt ELF-Versionstags pro Bibliothek. Statische Analyse
des verifizierten Linux-Artefakts auf dem Mac, keine dortige Linux-Ausführung.
42 Python-Prüfungen bestehen, eine Windows-Prüfung übersprungen; installierte
Mac-Paketimporte weiterhin geprüft. Neue Scanner-CI und transitive ABI-Abnahme
offen. [Laufzeitnachweis](../../docs/PAKET_LAUFZEIT.md).
Der neue ELF-Fixture besteht inzwischen nativ in beiden Linux-Python-Jobs
zu402adb5/CI38172648396; Desktop-/Paketprüfungen laufen noch, Mac-CI queued.
Der originale Linux64-Nachweis identifiziert Rust1.99.0/b940084d, bereits im
mitgelieferten Quellmanifest. Windows-/Linux-CI verlangen jetzt den Abgleich
von Compilercommit, Release, explizitem Ziel und Lizenztext-Hash.46 lokale
Python-Prüfungen bestehen, eine Windows-Prüfung übersprungen. Diese neue
CI-Grenze braucht noch ihren eigenen Lauf. [Vertrag](../../docs/RUST_RUNTIME_NACHWEIS.md).
In CI38172915154 zu d575caf bestehen inzwischen die Compiler-/Quellen-
Recordschritte in allen vier Windows-/Linux-Desktopjobs; Gesamt-/Paketjobs
laufen noch, Mac-Jobs queued.

Der tatsächliche Intel/macOS-Paketwechsel63→64→63 erhält20 Testdateien,
gespeicherte Notiz/Darstellung, Kontext-/Suchausgaben und vollständige Sicherung/
Wiederherstellung. Drei App-Aufnahmen angesehen. Neuer Prüfer und automatische
Wiederöffnungsprüfung desselben Pakets ergänzen den manuellen Updatevertrag.
Neue Windows-/Linux-/ARM64-Updateabnahmen bleiben offen.
Erweiterte lokale0.9.64-Paketprüfung besteht:20 unveränderte Dateien,126 Desktop-,
167 Tastatur-,75 Sicherungsbedingungen, zwei Einstellungsprozesse sowie CLI,
Runtime und Versionsidentität. Kein neuer Gesamtlauf der Produkt-CTest-Suite.
[Vertrag](../../docs/UPDATE_NACHWEIS.md).

## Neuer UI-Schritt0.9.65

Readeraktionen verwenden gemessene Inhaltsbreiten und zeigen ganze Wörter
oder vorhandene Icons. Bearbeiten/Speichern priorisiert, Namen/Reihenfolge/
Befehle erhalten.800 Bedingungen über36 Größen-/Schrift-/Zustandsszenarien
und10 passende Produktprüfungen bestehen (185,15s), Aufnahmen angesehen.
Kein neuer83er-Gesamtlauf. [Vertrag](../../docs/READER_AKTIONEN.md).
Sauberes65-Paket und Installation geprüft:98 Dateien paketgleich,18 eigene
Gedächtnisdateien beim Öffnen unverändert; tatsächliche Aufnahme angesehen.
126 Desktop-,167 Tastatur-,75 Sicherungsbedingungen,20-Datei-Wiederöffnung,
zwei Einstellungsprozesse, CLI-/Runtime-/Version bestehen. Echter Wechsel
64→65→64 erhält20 Testdateien und Einstellungen. Drei saubere Versions-/
Reader-Nachprüfungen bestehen (8,32s),47 Python-Prüfungen bestehen, eine native
Windows-Prüfung übersprungen. CI38174603923 zu1a19bee: Linux-Release samt Paket
erfolgreich; Windows/Linux-Debug in Arbeit, Mac queued. Die neue Windows-
Fixtureabnahme bleibt bis zum erfolgreichen Paket offen.

CI38174603923 zu1a19bee ist inzwischen in allen vier Windows-/Linux-Desktopjobs
einschließlich Release-Paketen erfolgreich. Damit sind Readerlayout und korrigierte
LF-Updatefixture dort nativ bestätigt. Mac-Jobs weiter queued.

Der Timingprüfer trennt nun gezeichnete Notiz, angenommenen Modellpfad,
Vorbereitungszustand und Übergangsende. Drei Metal-Läufe: lange Übergabe erstmals
vorbereitete Textkommandos484–785ms, übergangsfreies Bild632–913ms ab Notizaufruf.
Ein schneller Retarget erreicht keine Bereitschaft und bleibt ausdrücklich ohne
Meilenstein.19 Kopie-/18 Originaldateien unverändert; fünf serielle Messläufe,
CLI-/CSV-Schutz und50 Python-Prüfungen bestehen (eine Windows-Prüfung übersprungen).
App65 unverändert; nächster Leistungsansatz: Worker und erster Bereitschaftsframe.
[Vertrag und Grenzen](../../docs/FRAME_MESSUNG.md).

Alle vier Windows-/Linux-Desktopjobs zu d575caf/CI38172915154 bestehen
einschließlich Release-Paketen und neuer Compiler-/Lizenzquellengrenze.
Neuer Update-Prüfer zu a5a5127: Linux-Debug/Release und Paket sowie Windows-Debug
erfolgreich; Windows-Release-Paket scheitert am Prüfer-Textmodus (CRLF statt LF).
Fixture schreibt jetzt UTF-8-Bytes; Windows-Übersetzungsregression und erneuter
echter Mac-Wechsel bestehen. Neue native Paketabnahme folgt; Mac-CI queued.

## Geprüfter Fortschritt

0.9.63 veröffentlicht während des Ladens den sichtbaren Status als Readerkind,
keine vorzeitigen vollständigen Markdown-Textläufe. Danach wird der komplette
Dokumentinhalt veröffentlicht; frühe Scrollbefehle und native Rollen bleiben.
Negative Regression bestätigt; vier gezielte Tests und82/82 lokale Release-Tests
bestehen (447,15s). ASan/UBSan:120 Reader-,45 Frühscroll- und622 Mac-Bedingungen,
39 eigene C-Dateien/56 Hashes; externe UI nicht instrumentiert, Leaks aus, kein TSan.
[Vertrag](../../docs/LADEZUSTAND_NATIVE.md), Originale `build/reader-preflight-verification/`.

Drei serielle Metal-Paare: erstes Bild78,28→62,62ms,77,29→61,68ms,78,77→64,67ms;
mediane Verbesserung20,00Prozent. Quelle/Kopie unverändert. Kein menschlicher
Trackpad-/Eingabenachweis; erste fertige Textanzeige nicht unabhängig gemessen.
Ein Inline-Auftragscache ohne verlässlichen Gewinn wurde entfernt.

0.9.62 begrenzt visuelle Fokusrahmen auf die aktive Modalgruppe.0.9.61 erhält
Scrollbefehle während des Ladens und begrenzt sie nach Messung der Inhaltshöhe.
Hilfe kehrt mit Eingaben, Auswahl, Fokus, Meldung und Position zur Ursprungsaufgabe
zurück; normale Modalaufgaben erhalten Reader-/Editorpläne. Verträge in SOURCES.

## Aktuelle Plattformnachweise

CI38169362014 zu22a8504: Windows-Release und Linux-Debug/Release erfolgreich;
beide Release-Pakete tatsächlich geprüft. Windows-Debug114558512596 scheiterte
bei der Hilfe-Kontextprüfung: zwei Bilder garantieren keinen fertigen Reader.
Der Prüfer wartet jetzt begrenzt auf Bereitschaft und behält sämtliche Höhen-/
Rückkehrassertionen. Hilfe/Reader lokal bestehen (17,26s),94 Hilfebedingungen auch
unter ASan/UBSan mit bestätigten Bibliotheks-/Helperhashes. Alle vier Windows-/Linux-Desktopjobs zu798b84e bestehen im
Lauf38170374268 einschließlich Release-Paketen. Die installierte64-Fassung enthält zusätzlich Icons und Labelpriorität;
798b84e belegt den vorherigen63-Stand. Native Mac-CI zuletzt queued; lokale Intel-Prüfung separat. ARM64 und
frische Nutzerrechner offen. Frühere Erfolge ersetzen keine neue Prüfung.

## Verbleibende Release-Arbeit

Erste Quelle/Parser/Worker, native Veröffentlichung fertiger Blöcke, Rasterung,
Tabellen, Budgetrückfälle, riesige Absätze und Shutdown weiter prüfen. SBUI-074
bleibt für vollständige Leistungs-/Geräte-/Großdatenabnahme offen. Native Snapshot-
Pixelidentität und Material-Ersatzfläche bei Speicherfehler ebenfalls offen.

IME/VoiceOver/NVDA/Orca, native Dialoge, Monitore/Skalierungen/Langzeit, volle
Windows-/Linux-Zielvolumes und physische Persistenz, Mindest-OS/frische Rechner,
vollständige Runtime-/Lizenzprüfung, vertraulicher Sicherheitskanal, Signierung,
dauerhafte aktuelle Release-Pakete und abschließende natürliche Texte bleiben
Release-Arbeit. Mindest-OS-Vorschlag auf Nutzerantwort erarbeitet:14.0/Windows11 25H2/Ubuntu24.04;
noch zu bestätigen und nativ auf Mindestständen abnehmen.
Apple-Developer-Konto und Windows-Zertifikat fehlen. **1.0 nur nach Nutzerfreigabe.**
