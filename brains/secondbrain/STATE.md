# SecondBrain Aktueller Stand

Stand: 6. Oktober 2026. Diese Übersicht ist eine datierte Zusammenfassung;
aktuelle Nachweise und Grenzen stehen im [Umsetzungsstand](../../docs/STATUS.md).

## Belegter Stand

- Grundlagen und Apple UI-Recherche wurden vor dem UI-Entwurf dokumentiert.
- Der eigene C17-Kern verwaltet Projekte, Markdown, Suche, Archiv und Kontext.
- Die eigene Desktop-App bietet Anlegen, Lesen, Bearbeiten, Speichern, Quellen
  und Projektwechsel mit Schutz ungespeicherter Änderungen und Speicherkonflikte.
- Die [Abnahme zu 287ef48](https://github.com/Lulus792/SecondBrain/actions/runs/37445131476)
  besteht mit 18 Jobs. Desktop Debug/Release und entpackte Pakete bestehen auf
  Windows x64, macOS ARM64 und Linux x64.
- Lokal bestehen alle fünf C-/UI-Prüfungen auch mit AddressSanitizer und
  UndefinedBehaviorSanitizer auf Intel macOS 14.6.1.
- Ein tatsächliches macOS-Archiv besteht nach Entpacken und Verschieben denselben
  Bedienablauf. Das [Paketverfahren](../../docs/DISTRIBUTION.md) ist implementiert.
- Diese Wissensbasis wurde mit dem C-Kern angelegt und mit projektspezifischen
  Inhalten und relativen Originalquellen befüllt.

## Ergebnis der Umsetzung

Version 0.1 ist bereit für den lokalen Arbeitsablauf. Diese eigene Instanz ist in
der App geladen und betrachtet; ihre lokalen Quellen sind erreichbar. Der
Bedienablauf prüft auch Konfliktkopie, Archivierung und Speichern beim Beenden.
Der eigene Projektkontext wird in allen sechs Desktop-Jobs geöffnet und gerendert.
Die Intel-macOS-App liegt lokal unter dist/SecondBrain/secondbrain.app;
weitere Pakete stehen im verlinkten GitHub-Lauf als Actions-Artefakte bereit.

## Neue Umsetzung: Version 0.2

Lumen-Farben, eigene C-Glasdarstellung, reale Sternkarte und Kamerabedienung sind
in die App integriert. Die bisherigen Arbeitsabläufe verwenden weiter denselben
C-Kern. Sichtbarer Fokus, Tab, F6, Pfeile und Bestätigung erlauben reine Tastaturwege.
Der Editor hält seine Undo-Historie getrennt von Suche und Dialogen.
[Bedienvertrag und Prüfumfang](../../docs/UI_TASTATUR.md).

Lokal bestehen sieben Release-Prüfungen und dieselben sieben Prüfungen mit
AddressSanitizer/UndefinedBehaviorSanitizer. Der Tastaturdurchlauf prüft 85 Aussagen,
der bisherige Bedienweg 87. Kleine Fenster mit 150 Prozent Schriftgröße sind darin
enthalten. Die tatsächliche Darstellung wurde in groß und klein betrachtet. Die
entpackte Intel-macOS-App besteht beide Bedienwege aus einem anderen Ordner.
Die [Abnahme zu 5534ff0](https://github.com/Lulus792/SecondBrain/actions/runs/37466622105) besteht mit allen 18 Jobs.
Desktop Debug/Release mit je sieben Prüfungen und entpackte Pakete mit beiden
Bedienwegen bestehen auf Windows x64, macOS ARM64 und Linux x64. Der oben
verlinkte Lauf zu 287ef48 belegt weiterhin die erste Version.

Version 0.2 ist damit für den beauftragten lokalen Arbeitsablauf abgenommen.
Die aktuelle Intel-App liegt unter dist/SecondBrain/secondbrain.app; die weiteren
Pakete stehen im neuen GitHub-Lauf. Neue Erweiterungen werden anhand der
offenen Fragen geplant.

## Verfeinerung in Version 0.2.1

Direkte Pfeilnavigation, kurze Kamera- und Scrollübergänge und verbesserte
Ausrichtung sind implementiert. Die README hat nach Recherche von neun GitHub-
Projekten einen neuen Einstieg mit tatsächlicher App-Vorschau. Die Arbeitsfassung
enthält nur das eigene Projektgedächtnis und neutrale Beispiele.

Lokal bestehen sieben Release-Prüfungen (93 Maus- und 99 Tastaturaussagen), dieselben
sieben Tests mit AddressSanitizer/UndefinedBehaviorSanitizer und neun Python-Prüfungen.
Das entpackte Intel-macOS-Paket besteht beide Bedienwege. Die [Abnahme zu 29b15d1](https://github.com/Lulus792/SecondBrain/actions/runs/37479319260)
besteht mit allen 18 Jobs einschließlich Debug/Release und entpackter Pakete auf
Windows x64, macOS ARM64 und Linux x64. Der [Umsetzungsstand](../../docs/STATUS.md)
beschreibt Umfang und Grenzen. Die [Release-Liste](../../docs/RELEASE.md)
ordnet die Lücken vor 1.0; die Endprodukt-Texte werden abschließend erst vor dem
vollständigen Release geprüft.

## Aktives Ziel vor 1.0

Der Nutzer hat UI-Fehler, Raumfahrt, Menünavigation und die übrigen Release-Arbeiten
beauftragt. Die Version 1.0 ist bis zu seiner ausdrücklichen Freigabe gesperrt.
Der erste Entwicklungsschritt 0.3.0 implementiert Archiv-Rückkehr, eigene Scrollgrenzen,
Weltpositions-Fahrten, große Leseansicht, feste Schließen-Knöpfe und eigene Icons.
Sieben lokale Release-Prüfungen bestehen mit 126 Maus- und 105 Tastaturaussagen;
kleine Ansichten mit 200 Prozent Schrift wurden betrachtet. Die eigene MIT-Lizenz
ist gewählt. Dieselben sieben
Tests bestehen mit AddressSanitizer/UndefinedBehaviorSanitizer. Das entpackte
Intel-macOS-Paket besteht beide Bedienwege. Die [Abnahme zu c224223](https://github.com/Lulus792/SecondBrain/actions/runs/37494305314) besteht mit allen 18 Jobs einschließlich Desktop Debug/Release und entpackter
Pakete auf Windows x64, macOS ARM64 und Linux x64. Umfang und Grenzen nennt der
[Umsetzungsstand](../../docs/STATUS.md).

Das Gesamtziel bleibt aktiv. Nächste Arbeiten sind Sicherung/Wiederherstellung,
dauerhafte Einstellungen, native Ordnerwahl, Screenreader-Anbindung, weitere
Daten- und Leistungsabnahme sowie Distribution. Die vollständige [Release-Liste](../../docs/RELEASE.md)
bleibt maßgeblich. Apple-Developer-Konto und Windows-Zertifikat fehlen noch;
Signierungswege werden vorbereitet, tatsächliche Signierung bleibt eine externe Abnahme.

## Grenzen

Keine native Screenreader-Anbindung, automatische Synchronisation oder integrierte
Chat-Anbieter. Grundlegende Markdown-Darstellung; Editor-Undo und Schriftabdeckung
sind begrenzt. Pakete sind nicht durch Apple notarisiert.
Details: [Distribution](../../docs/DISTRIBUTION.md).


## Neuer Entwicklungsschritt 0.3.1

Dauerhafte Darstellung und letzte Projektposition sowie native Ordnerauswahl sind
implementiert. Konfiguration wird bei beschädigten Daten oder erkannten parallelen
Änderungen erhalten. Ein fehlender letzter Ordner lässt sich bewusst ersetzen;
verspätete Dialogantworten verändern kein neues Formular. [Details](../../docs/EINSTELLUNGEN.md).
Zehn lokale Release-Prüfungen und der vollständige Sanitizer-Lauf bestehen,
zusätzlich die drei betroffenen Prüfungen nach der letzten kleinen Korrektur.
51 neue UI-Aussagen und zwei getrennte App-Prozesse prüfen die Einstellungen.
Plattformnachweise für diesen Stand folgen erst nach ausgeführten CI-Jobs.
Sicherung, native Zugänglichkeit, erster Start, OS-Vorgaben, Leistungsprüfung und
Distribution bleiben Arbeiten des aktiven Ziels. Version 1.0 bleibt gesperrt.


Die erste Crossplatform-Prüfung zu 66760a3 scheitert in den beiden Linux-Desktop-
Jobs, während der C-Kern auf allen drei Systemen besteht. Fehlende öffentlich
zugängliche Detailprotokolle werden durch neue CI-Fehlerannotationen ergänzt.
Zusätzlich wartet die Einstellungsanbindung nun mit SDL_SyncWindow auf asynchrone
Fenstergrößen. Die drei betroffenen Prüfungen bestehen lokal in Release und mit
ASan/UBSan. Der erneute Linux-Nachweis folgt erst nach tatsächlich bestandenem Lauf.


## Abnahme 0.3.1 nach der Fensterkorrektur

Der [Lauf zu 259fca1](https://github.com/Lulus792/SecondBrain/actions/runs/37499338633) besteht mit allen 18 Jobs.
Je zehn Desktop-Prüfungen bestehen in Debug und Release auf Windows x64,
macOS ARM64 und Linux x64. Alle drei entpackten Pakete bestehen Mausbedienung,
reine Tastaturbedienung und zwei getrennte App-Prozesse mit gespeicherten
Einstellungen. Die neue UI-Prüfung enthält 51 Aussagen. Der frühere Linux-Fehler
ist durch die explizite Synchronisierung der Fenstergröße behoben; beide
Linux-Profile bestehen im Korrekturlauf.

Lokal besteht das neu gepackte und verschobene Intel-macOS-Paket ebenfalls mit
126 Maus- und 105 Tastaturaussagen sowie dem Prozessneustart. Die installierte App
unter dist/SecondBrain/secondbrain.app ist aktualisiert; otool -L zeigt nur
macOS-Systembibliotheken. Die Tests verwenden versteckte native Fenster und
SDL-Softwaredarstellung. Tatsächliche OS-Dialoge, Screenreader, reale GPU-/Display-
Umgebungen und Langzeitsitzungen bleiben gesonderte offene Abnahmen.
