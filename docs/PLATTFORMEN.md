# Plattformprüfung des SecondBrain

Windows, macOS und Linux sind verbindliche Zielplattformen. Der vorhandene
Strukturprototyp benötigt Python ab 3.10 und verwendet ausschließlich dessen Standardbibliothek.
Markdown und JSON werden in UTF-8 geschrieben; generierte Dateien verwenden LF.

Die [eigene Anwendung](PROJEKTPLAN.md) wird in C umgesetzt. Die folgenden
Prototypnachweise betreffen Python; die native C-Prüfung steht im eigenen Abschnitt
weiter unten. Desktop- und Paketnachweise stehen in einem eigenen Abschnitt.

## Prüfungsumfang

Die Tests prüfen Erstellung, unabhängige Projektinstanzen, Unicode-Pfade,
relative Projektverweise, absolute Verweise bei unterschiedlichen Laufwerken,
ungültige Kennungen und den Erhalt vorhandener Dateien und Verzeichnisse.
Ein Kommandozeilentest startet den Generator aus einem fremden Arbeitsordner
und kontrolliert zusätzlich, dass Originaldateien im verknüpften Projekt
unverändert bleiben.

Symlink-Schutz wird geprüft, wenn das Betriebssystem deren Erstellung erlaubt.
Ohne entsprechende Windows-Rechte wird nur dieser Test mit einer ausgewiesenen
Begründung ausgelassen.

## Nachweise

Am 6. Oktober 2026 bestehen lokal 9/9 Tests mit Python 3.14.0 auf macOS 14.6.1;
der Symlink-Test wurde ausgeführt.

Der [GitHub-Lauf für `58aac00`](https://github.com/Lulus792/SecondBrain/actions/runs/37431461978)
besteht am selben Tag mit sechs erfolgreichen Jobs:

| Runner | Python 3.10 | Python 3.14 |
| --- | --- | --- |
| windows-latest | bestanden | bestanden |
| macos-latest | bestanden | bestanden |
| ubuntu-latest | bestanden | bestanden |

Im [ersten Lauf](https://github.com/Lulus792/SecondBrain/actions/runs/37431163324)
scheitern beide Windows-Jobs. Das Lesen der UTF-8-Metadaten ohne angegebene
Kodierung lässt sich lokal mit einem simulierten CP1252-Standard als Fehler
reproduzieren. Die Tests lesen und schreiben ihre Textdateien jetzt ausdrücklich
als UTF-8; der Generator verwendet bereits UTF-8. Nach der Korrektur bestehen
die lokale Simulation und alle sechs CI-Jobs.

Die Runner-Bezeichnungen bezeichnen die tatsächlich verwendete CI-Matrix.
Sie sind keine Zusage für jede historische Windows-, macOS- oder Linux-Version.

## Native C Prüfung

Der [Lauf zu 4c99bb3](https://github.com/Lulus792/SecondBrain/actions/runs/37435200479)
besteht am 6. Oktober 2026 mit sechs erfolgreichen C17-Jobs: Windows, macOS und
Ubuntu, jeweils Debug und Release. Dabei werden die C-Bibliothek und das native
Kommandozeilenwerkzeug gebaut. Der Dateiablauf prüft Erstellung, Wiederöffnung,
UTF-8, Suche, Speicherkonflikte, Archivierung, KI-Kontext und Metadaten.

Lokal bestehen außerdem Kern- und UI-Grundlagenprüfung mit AppleClang 16 auf
Intel macOS 14.6.1 sowie zusätzliche Läufe mit AddressSanitizer und
UndefinedBehaviorSanitizer. Die UI-Prüfung rendert mit dem nativen Cocoa-Fenster
und SDL-Softwarerenderer. Dieser frühere Lauf belegt Renderer und Editorbausteine;
die Desktop-Prüfung ist davon getrennt.

## Native UI Grundlagenprüfung

Der [Lauf zu 2c1a911](https://github.com/Lulus792/SecondBrain/actions/runs/37437068287)
besteht ebenfalls. Die drei UI-Jobs bauen SDL3 und das UI-Ziel und prüfen
Renderer, Schriften, Texteingaben, Unicode-Auswahlersetzung sowie Rückgängig und
Wiederholen. Linux verwendet X11 unter Xvfb, Windows und macOS ihre nativen
Fenstersysteme; die Tests rendern über den SDL-Softwarerenderer in einem versteckten
Testfenster. Die Kern- und Python-Jobs dieses Laufs bestehen ebenfalls.

Diese Ergebnisse belegen die verwendeten Bausteine. Die Projektoberfläche wird
zusätzlich durch vollständige Bedien- und Paketprüfungen nachgewiesen.

## Desktop und entpackte Pakete

Der [Lauf zu 287ef48](https://github.com/Lulus792/SecondBrain/actions/runs/37445131476)
besteht am 6. Oktober 2026 mit 18 erfolgreichen Jobs. Er prüft den C-Kern
und die Desktop-App in Debug/Release auf allen drei Systemen und bewahrt die
älteren Python-Prüfungen. Die sechs Desktop-Jobs führen je fünf Prüfungen aus:
Renderer/Editor, vollständiger Bedienablauf, eigene Projektinstanz,
Anwendungszustand und C-Kern. Alle drei Release-Jobs entpacken das tatsächliche
Anwendungsarchiv und starten daraus denselben Bedienablauf.

| Plattform | Desktop Debug | Desktop Release | Entpacktes Paket |
| --- | --- | --- | --- |
| Windows x64 | bestanden | bestanden | bestanden |
| macOS ARM64 | bestanden | bestanden | bestanden |
| Ubuntu Linux x64, X11/Xvfb | bestanden | bestanden | bestanden |

Der Desktop-Ablauf steuert die tatsächlichen Bedienelemente über SDL-Ereignisse.
Er umfasst Projekt-/Notizerstellung, Unicode-Bearbeitung, Speichern und Wiederöffnen,
Suche, Wechsel mit Speichern/Verwerfen/Abbrechen, schreibgeschützte Quellen,
Kontext und Zwischenablage, Konfliktkopie, Archivierung und Speichern beim Beenden.
Die eigene Projektinstanz wird aus dem Repository geöffnet und tatsächlich gerendert.

Lokal bestehen alle fünf Prüfungen mit AddressSanitizer und
UndefinedBehaviorSanitizer auf Intel macOS 14.6.1. Tatsächliche Renderbilder
wurden in groß/hell und klein/dunkel mit 150 Prozent Schrift betrachtet,
einschließlich langer Dokumenttitel. Die CI veröffentlicht Testbilder und
Protokolle getrennt von den Anwendungspaketen.

Die [Distribution](DISTRIBUTION.md) dokumentiert Paketaufbau und Start.
Lokal zeigt `otool -L` ausschließlich macOS-Systembibliotheken. Die UI wird in der
CI in versteckten nativen Testfenstern mit SDL-Softwarerenderer geprüft;
dies ist kein manueller Test jeder Desktop-Umgebung oder GPU-Konfiguration.

## Abnahme von Version 0.2 mit Lumen und Tastatur

Der [Lauf zu 5534ff0](https://github.com/Lulus792/SecondBrain/actions/runs/37466622105)
besteht am 6. Oktober 2026 mit 18 erfolgreichen Jobs. Er belegt die Integration
von Sternkarte, eigener Glasdarstellung und vollständigen Tastaturwegen.

| Plattform | Desktop Debug, 7 Tests | Desktop Release, 7 Tests | Entpacktes Paket, beide Bedienwege |
| --- | --- | --- | --- |
| Windows x64 | bestanden | bestanden | bestanden |
| macOS ARM64 | bestanden | bestanden | bestanden |
| Ubuntu Linux x64, X11/Xvfb | bestanden | bestanden | bestanden |

Je Desktop-Job werden Renderer/Editor/Material, bisherige Bedienung, reine
Tastaturbedienung, eigene Projektinstanz, echte Graphverweise, Anwendungszustand
und C-Kern geprüft. Der reine Tastaturdurchlauf umfasst 85 Aussagen, einschließlich
aktiver Texteingabe, Undo/Redo, Sternwahl, Schutzdialog, Quellen, 300 Links,
kleinem Fenster mit 150 Prozent Schrift, Kontext, Konfliktkopie und Archiv.
Der bisherige SDL-Bedienweg umfasst 87 Aussagen.

Jeder Release-Job erstellt und entpackt sein eigenes Anwendungsarchiv und führt
daraus beide Bedienwege mit den mitgelieferten Ressourcen aus. Die drei Pakete
sind als Actions-Artefakte verfügbar. Lokal bestehen die entsprechenden Prüfungen
auch auf Intel macOS 14.6.1 mit Sanitizern sowie das entpackte Intel-Paket.
Tatsächliche große und kleine App-Renderbilder wurden lokal betrachtet und korrigiert.
Die CI prüft versteckte native Fenster mit SDL-Softwarerenderer; sie belegt
keine manuelle Abnahme aller GPU-Treiber oder Desktop-Umgebungen.

## Grenzen der Nachweise

Die Prüfungen belegen die jeweils benannten Abläufe. Der KI-Kontext wird erzeugt
und kopiert; ein externer KI-Anbieter wurde dabei nicht integriert.

Wissensbasen bleiben bei einem Betriebssystemwechsel lesbar. Lokale Projektpfade
müssen am Zielrechner erreichbar sein. Absolute Laufwerksverweise sind an den
jeweiligen Rechner gebunden; relative Verweise bleiben bei gemeinsamem
Verschieben des Projekts und seiner Wissensbasis nutzbar.

## Abnahme von Version 0.2.1

Der [Lauf zu 29b15d1](https://github.com/Lulus792/SecondBrain/actions/runs/37479319260) ist am 6. Oktober 2026 mit allen 18 Jobs erfolgreich abgeschlossen. Alle sechs
Desktop-Jobs bestehen mit jeweils sieben Tests auf Windows x64, macOS ARM64 und
Linux x64 in Debug/Release. Alle drei entpackten Release-Pakete bestehen den
Maus-Bedienweg mit 93 und den reinen Tastaturweg mit 99 Aussagen. Geprüft werden
zusätzlich direkte Pfeilnavigation, Schutz offener Entwürfe, Zwischenstände der
Kamerabewegung, weiches Scrollen, kleine Mausradschritte und reduzierte Bewegung.

Auf Intel macOS 14.6.1 bestehen lokal alle sieben Tests in Release und mit
AddressSanitizer/UndefinedBehaviorSanitizer. Das Archiv SecondBrain-0.2.1-Darwin-x86_64
besteht nach Entpacken in einen anderen Ordner beide Bedienwege. Große und kleine
Renderbilder mit 150 Prozent Schriftgröße wurden betrachtet. Die oben genannten
Grenzen der Software- und Geräteabnahme gelten weiterhin.

## Entwicklungsschritt 0.3.0

Die [Abnahme zu c224223](https://github.com/Lulus792/SecondBrain/actions/runs/37494305314) besteht am 6. Oktober 2026 mit allen 18 Jobs. Die sechs nativen Desktop-Jobs
bestehen mit je sieben Tests auf Windows x64, macOS ARM64 und Linux x64 in Debug
und Release. Die drei entpackten Release-Pakete bestehen beide Bedienwege mit
126 Maus- und 105 reinen Tastaturaussagen.

Lokal auf Intel macOS 14.6.1 bestehen dieselben sieben Release- und sieben
Sanitizer-Tests. Das entpackte Archiv SecondBrain-0.3.0-Darwin-x86_64 besteht beide
Bedienwege einschließlich Ressourcen und MIT-Lizenz aus einem anderen Ordner.
Kleine Darstellungen mit 200 Prozent Schrift sind geometrisch geprüft und betrachtet.
Die Tests belegen die benannten Abläufe mit versteckten SDL-Softwarerenderer-Fenstern.
Das aktive Gesamtziel und die übrigen Release-Abnahmen bleiben offen.


## Version 0.3.1: Einstellungen und Ordnerwahl

Der [Lauf zu 259fca1](https://github.com/Lulus792/SecondBrain/actions/runs/37499338633) besteht am 6. Oktober 2026
mit 18 erfolgreichen Jobs. Die Artefaktnamen belegen Windows X64, macOS ARM64
und Linux X64. Alle sechs Desktop-Jobs bestehen je zehn Prüfungen in Debug/Release.
Dazu gehören Format-/Bestandsschutz für Einstellungen, neu erzeugte Fenster,
51 UI-Aussagen und zwei getrennte App-Prozesse. Die drei entpackten Release-Pakete
bestehen beide Bedienwege und Prozessneustart mit isolierter Konfiguration.

| Plattform | Desktop Debug, 10 Tests | Desktop Release, 10 Tests | Entpacktes Paket |
| --- | --- | --- | --- |
| Windows x64 | bestanden | bestanden | beide Bedienwege und Neustart bestanden |
| macOS ARM64 | bestanden | bestanden | beide Bedienwege und Neustart bestanden |
| Ubuntu Linux x64, X11/Xvfb | bestanden | bestanden | beide Bedienwege und Neustart bestanden |

Die [erste Abnahme zu 66760a3](https://github.com/Lulus792/SecondBrain/actions/runs/37498493340)
scheiterte nur in den beiden Linux-Desktopprofilen. Neue CTest-Fehlerannotationen
belegen den Größenvergleich beim Einstellungsneustart. SDL führt X11-Größenänderungen
asynchron aus; der Korrekturstand wartet ausdrücklich auf deren Abschluss.
Die erste fehlgeschlagene Prüfung bleibt als Nachweis der Korrektur erhalten.

Lokal bestehen zehn Release-Prüfungen und ein vollständiger Sanitizer-Lauf;
nach den letzten Änderungen bestehen zusätzlich die drei betroffenen Prüfungen
in Release und ASan/UBSan. Das tatsächliche neue Intel-Paket besteht 126 Maus-,
105 Tastaturaussagen und getrennte Prozessneustarts. Kleine Bilder bei 150/200
Prozent Schrift wurden betrachtet. Native OS-Dialoge und Screenreader wurden
nicht interaktiv bedient; kein solcher Plattformnachweis wird daraus abgeleitet.


## Version 0.4.0: Sicherung und Wiederherstellung in der App

Der [Lauf zu d2d9c2b](https://github.com/Lulus792/SecondBrain/actions/runs/37508856383) besteht am 6. Oktober 2026
mit allen 18 Jobs. Artefaktnamen bestätigen Windows X64, macOS ARM64 und Linux X64.

| Plattform | Desktop Debug, 14 Tests | Desktop Release, 14 Tests | Entpacktes Paket |
| --- | --- | --- | --- |
| Windows x64 | bestanden | bestanden | Maus/Tastatur/Sicherung, Neustart und CLI bestanden |
| macOS ARM64 | bestanden | bestanden | Maus/Tastatur/Sicherung, Neustart und CLI bestanden |
| Ubuntu Linux x64, X11/Xvfb | bestanden | bestanden | Maus/Tastatur/Sicherung, Neustart und CLI bestanden |

Neu enthalten sind 487 Integritätsaussagen, 27 injizierte Schreibfehleraussagen,
Produktions-CLI-Prozesse und 75 Sicherungs-Bedienaussagen mit 8-MiB-Anhang.
Der UI-Ablauf prüft Vorschau-Bindung, erhaltene Entwürfe, belegte Namen,
Save-Konflikt, falsche/späte Auswahlantworten, Abbruch und Beenden. Dateidialog-
Rückgaben werden simuliert; die tatsächlich nativen OS-Dialoge wurden nicht bedient.

Lokal bestehen entpacktes Intel-Paket, 14 Releaseprüfungen und gezielte Nachprüfungen.
Zwölf Prüfungen des Sanitizer-Gesamtlaufs bestanden; zwei durch parallel gestartete
Clipboard-Tests gestörte Wege bestehen seriell wiederholt mit ASan/UBSan.
Kleine Darstellungen bei 200 Prozent Schrift und echte Hintergrundarbeit wurden
als Softwarebilder betrachtet. Fehler-Injektion ist ausdrücklich kein physisch
volles Volume. Native Screenreader, reale GPU-/Display- und Langzeitnachweise fehlen.


## 0.5.0: native Zugänglichkeitsadapter

Der [Lauf zu 0c9fc8b](https://github.com/Lulus792/SecondBrain/actions/runs/37512645717) ist erfolgreich abgeschlossen: 18 Jobs.
Je 15 Desktoptests bestehen in Debug/Release auf Windows x64, macOS ARM64 und
Linux x64. Die drei tatsächlich entpackten Release-Pakete bestehen die bisherigen
Maus-/Tastatur-/Sicherungswege, Einstellungsneustart und Produktions-CLI-Sicherung.
Damit wird unter Windows auch das Laden der ausgelieferten AccessKit-DLL geprüft.

Der Test native-accessibility benutzt auf macOS tatsächlich NSAccessibility:
Rollen, Namen, Textwerte, Press-Aktionen, Auswahl und Lesewerte. Auf Windows/Linux
prüft er in dieser Fassung den gemeinsamen Snapshot-/Aktionsvertrag; eine native
UIA-/AT-SPI-Clientabfrage wird damit nicht behauptet. Menschliche VoiceOver/NVDA/
Orca-Abnahme bleibt auf allen Systemen offen.

Lokal auf Intel macOS 14.6.1 bestehen alle 15 Release-Tests in 168,90 Sekunden;
die Zugänglichkeitsprüfung umfasst 107 Aussagen. CPack erzeugt das Intel-Paket
mit der korrekten Bundle-Version und allen vier unveränderten AccessKit-Hinweisen.
Der folgende Sanitizer-/Paketprüfstand wird nach seinem Abschluss ergänzt.


### Lokale Abschlussprüfung 0.5.0

Alle 15 Tests bestehen auch im seriellen ASan/UBSan-Lauf (318,88 Sekunden),
einschließlich 107 Zugänglichkeitsaussagen. Der Anwendungscode ist instrumentiert;
die vorgebaute AccessKit-Bibliothek ist es intern nicht. Der entpackte Intel-Mac-
Build startet aus einem verschobenen Unicode-Pfad und rendert das eigene Projekt;
das tatsächliche Bild wurde betrachtet. Die entpackte Produktions-CLI besteht
den Sicherungs-/Prüf-/Wiederherstellungsablauf. Die lokale App unter
`dist/SecondBrain/secondbrain.app` ist auf 0.5.0 aktualisiert.

Paket SHA-256 (SecondBrain-0.5.0-Darwin-x86_64.tar.gz):
`3261e6420d9d49caf825318cf435f6e990fd0dd76f720299a04f3d6ce9203bed`.
Dies ist keine neue vollständige lokale Paket-Bedienabnahme: alle vollständigen
entpackten Paketwege sind im oben genannten Crossplatform-Lauf belegt. Der
komplette Release-Auftrag bleibt aktiv; die Versionsnummer 1.0 bleibt gesperrt.


## Native Clientabnahme auf allen drei Systemen

Der [korrigierte Lauf zu 8ff50ee](https://github.com/Lulus792/SecondBrain/actions/runs/37515016302) ist erfolgreich abgeschlossen: 18 Jobs,
15 Desktoptests je Debug/Release auf Windows x64, macOS ARM64 und Linux x64,
plus die drei tatsächlich entpackten Pakete. Der neue Test fragt Windows UIA
beziehungsweise Linux AT-SPI tatsächlich ab: benannte Schaltflächen und Rollen,
Unicode-Titel setzen/lesen, Notiz anlegen, Editorwert ändern/lesen und speichern.
Die App-Zustände werden danach geprüft. macOS behält seine native NSAccessibility-
Providerprüfung mit Auswahl und Schutz verspäteter Aktionen.

Linux verwendet eine private D-Bus-Sitzung mit in-memory GSettings; der Testclient
sucht ausschließlich seine eigene Prozesskennung. libatspi/GObject gehören nur
zum Testziel. Das Anwendungspaket erhält dadurch keine neue Prüfbibliothek.
Die explizite GObject-Verlinkung behebt den anfänglichen Linux-Testbuild. Die
vorherigen fehlgeschlagenen Läufe bleiben erhalten und werden nicht als bestanden
gewertet. Windows-/macOS-Erfolge des ersten Clientlaufs sind separate Nachweise.

Dies belegt die genannten nativen Abfragen und Aktionen, keine menschliche
VoiceOver/NVDA/Orca-Abnahme. Passive Dialog-/Hilfetexte, graphemgenaue Textläufe,
Zeilengeometrie, zurückhaltende Fortschrittsansagen und große animierte Dokumente
bleiben offen. Ebenso OS-Vorgaben, Schrift-Fallback und übrige Release-Aufgaben.
Die Produktversion bleibt 0.5.0; 1.0 bleibt bis zur Nutzerfreigabe gesperrt.


## 0.5.1: abgeschlossene native Nachprüfung

Der [korrigierte Lauf zu 9eeb402](https://github.com/Lulus792/SecondBrain/actions/runs/37518758088) ist mit 18 Jobs erfolgreich abgeschlossen:
15 Desktoptests je Debug/Release auf Windows x64, macOS ARM64 und Linux x64,
einschließlich der tatsächlichen Provider-/Clientabfragen von Titel und Hilfezeile.
Alle drei entpackten Pakete bestehen die bisherigen Maus-, Tastatur-, Sicherungs-,
Neustart- und Produktions-CLI-Wege. Die früheren Fehlerläufe bleiben erhalten.

Lokal bestehen alle 15 Release-Prüfungen (232,23 Sekunden) vor der letzten nativen
Korrektur und die gezielte abschließende Providerprüfung (121 Aussagen).
Alle 15 ASan/UBSan-Prüfungen bestehen für den Zwischenstand mit Titelgeometrie
(363,94 Sekunden). Nach lesbarem Textwert und Rollen-Anpassung bestehen zusätzlich
native-accessibility, keyboard-workflow und backup-ui-workflow mit ASan/UBSan
(267,30 Sekunden). Die vorgebaute UI-Bibliothek ist intern nicht instrumentiert.

Das endgültige Intel-Mac-Paket startet aus einem verschobenen Unicode-Pfad,
rendert das eigene Projekt und besteht den Produktions-CLI-Sicherungsablauf.
Das tatsächliche Bild wurde betrachtet. Die App unter dist/SecondBrain ist auf
0.5.1 aktualisiert. Paket-SHA-256:
`c22d00036a8d450711357a270a3d7f11a06ab4079487cc3708857910240f498a`.
Dies behauptet keine zusätzliche vollständige lokale Paket-Bedienabnahme;
die vollständigen Paketwege sind im genannten Crossplatform-Lauf belegt.

Offen: vollständige Dokumentstruktur, Grapheme/Zeilen, Fortschrittsansagen,
Schrift-Fallback/OS-Vorgaben und menschliche assistive Bedienung. Der Linux-
Clientlauf zeigt trotz erfolgreicher Abfragen AT-SPI-Cache-Signaturwarnungen.
Die Cache-/Signal-Kompatibilität muss getrennt untersucht werden; direkte
Abfragen beweisen sie nicht. Die gesamte Release-Liste bleibt aktiv; 1.0 bleibt gesperrt.


## 0.5.2: AT-SPI-Cachekorrektur abgenommen

Der [Lauf zu 8bcc048](https://github.com/Lulus792/SecondBrain/actions/runs/37522537774) besteht mit allen 18 Jobs: 15 Desktoptests je
Debug/Release auf Windows x64, macOS ARM64 und Linux x64 und alle drei entpackten
Pakete. Die zusätzliche Linux-Prüfung beobachtet echte Cache-Signale und GetItems,
während der Client Änderungsevents verarbeitet und seinen Cache nicht vor jeder
Abfrage leert. Release: 45 Add-/41 Remove-Signale, 0 falsche Signaturen, gültige
Sammelantwort. Debug: 46 Add-/41 Remove-Signale, ebenfalls 0 falsche Signaturen
und gültige Sammelantwort. Der ursprüngliche Adapter mit derselben Wire-Regression
lieferte 87 falsche Signaturen. Dieser Fehler ist damit gezielt nachgeprüft.

Linux baut die festgelegte UI-Abhängigkeit mit einer Korrektur von zwei Signal-
aufrufen. Cargo/Rust ab 1.87 ist eine Linux-UI-Buildvoraussetzung; fertige Pakete,
macOS-/Windows-Builds und der reine C-Kern benötigen sie nicht. Die UI-Quelle und
ihre Versionen sind über Archiv-/Quellhashes und Cargo --locked festgelegt.
Lokale Prüfungen bestätigen wiederholbaren Patch, Pfade mit Leerzeichen,
Ablehnung unbekannter Quelländerungen, den Lockgraph sowie die Compilerprüfung
der geänderten UI-Bibliothek. Die tatsächliche Linux-Abnahme erfolgt im obigen Lauf.

Der lokale Intel-Mac-Provider besteht weiterhin (121 Aussagen). Das entpackte
Intel-Paket startet aus einem Unicode-Pfad, und sein tatsächliches Bild wurde
betrachtet. dist/SecondBrain ist auf 0.5.2 aktualisiert. Paket-SHA-256:
`84bea47339b7b9e2453da706cb1bc3b038e71960277eaa149fb7db66e105e719`.
Die gesamte lokale Desktop-/Sanitizerabnahme wird hier nicht erneut behauptet;
die unveränderten Mac-Produktpfade sind zuvor geprüft, die neue Linux-Korrektur
ist durch native CI und Paketprüfung belegt.

Weitere Eventtypen und menschliche assistive Navigation bleiben eigene Abnahmen.
Dokumentstruktur, Grapheme/Zeilen, Fortschrittsansagen, OS-Vorgaben, Schrift-Fallback
und übrige Release-Aufgaben bleiben aktiv. 1.0 bleibt bis zur Nutzerfreigabe gesperrt.


## 0.6.0: Native Systemvorgaben

[Lauf zu 444471d](https://github.com/Lulus792/SecondBrain/actions/runs/37527601503):
18 erfolgreiche Jobs, 16 Desktoptests je Debug/Release auf Windows x64, macOS
ARM64 und Linux x64 sowie drei entpackte Pakete. Der neue system-appearance-Test
prüft Policy und getrennt gespeicherte eigene Werte. macOS/Windows lesen native
Vorgaben; Linux prüft echte D-Bus-Anfragen, laufende Änderung, Fehlererhalt und
Erholung mit einem privaten Testportal. Dies belegt keinen interaktiven Wechsel
aller Desktopbackends oder persönliche OS-Einstellungen.

Lokal auf Intel macOS 14.6.1: alle 16 Release-Tests (191,75 Sekunden) und alle
16 ASan/UBSan-Tests (398,57 Sekunden), einschließlich 21 Systemdarstellungs- und
121 nativer Zugänglichkeitsaussagen. Tatsächliche helle/dunkle Kontrastbilder
wurden betrachtet; nicht ausgewählte Sterne und Kanten bleiben zu blass. Die
vollständige Kontrast- und Geräteabnahme bleibt offen.


Das entpackte Intel-Paket 0.6.0 besteht lokal mit 126 Desktop-, 105 Tastatur- und
75 Sicherungs-UI-Aussagen sowie zwei Einstellungsprozessen und dem CLI-Sicherungs-
ablauf in Unicode-Pfaden. dist/SecondBrain ist auf 0.6.0 aktualisiert; der Start
mit dem eigenen Projektgedächtnis und das tatsächliche Bild sind geprüft.
Archiv-SHA-256: `897c24cea9a2bea02575c60afd7697d6ed73dce7da07d9a16fe43d84c50f173e`.
Die Pakete bleiben Entwicklungspakete ohne Herausgeberzertifikat. 1.0 bleibt gesperrt.


## Abnahme der Kontrastnacharbeit 0.6.1

[Lauf zu b064bd5](https://github.com/Lulus792/SecondBrain/actions/runs/37529776082)
besteht mit allen 18 Jobs: 16 Desktoptests je Debug/Release auf Windows x64,
macOS ARM64 und Linux x64 sowie drei entpackte Pakete. Lokal besteht das entpackte
Intel-Paket mit 126 Desktop-, 105 Tastatur- und 75 Sicherungs-UI-Aussagen sowie
Einstellungsprozessen und CLI-Sicherung. Sein tatsächlicher Start wurde betrachtet.
dist/SecondBrain ist auf 0.6.1 aktualisiert. Archiv-SHA-256:
`77dc5b20ccbd95d9a50bb652a59d937952eeba10e418cf08b0846dcc4055cee8`.
Vollständige assistive Bedienung und Kontrastabnahme aller Zustände bleiben offen.


## Abnahme des Einstiegs 0.7.0

[Lauf zu beba1e5](https://github.com/Lulus792/SecondBrain/actions/runs/37531214816)
besteht mit allen 18 Jobs: 17 Desktoptests je Debug/Release auf Windows x64,
macOS ARM64 und Linux x64 sowie drei entpackte Pakete. Lokal besteht das entpackte
Intel-Paket mit 126 Desktop-, 105 Tastatur- und 75 Sicherungs-UI-Aussagen sowie
Einstellungsprozessen und CLI-Sicherung. Archiv-SHA-256: `335f869ca5f4e3403ea21fa54abc148245d84b2e241e1dd846d4a2d71bb08b1e`.
Menschliche assistive Bedienung und echte native Dialogbedienung bleiben offen.


## Abnahme von 0.7.1

[Lauf zu b3c0af5](https://github.com/Lulus792/SecondBrain/actions/runs/37532681504)
besteht mit allen 18 Jobs: 18 Desktoptests je Debug/Release auf Windows x64,
macOS ARM64 und Linux x64 sowie drei entpackte Pakete. Lokal besteht das entpackte
Intel-Paket mit 126 Desktop-, 105 Tastatur- und 75 Sicherungs-UI-Aussagen sowie
Einstellungsprozessen und CLI-Sicherung. Archiv-SHA-256: `6bd2cea886e7e6f88335b8987cd1e25839f3b0b54ff969e84afe403114d24aba`.
Der weitere Release-Auftrag einschließlich Einzelprojekt-Fehlerzuständen bleibt aktiv.


## Abnahme von 0.7.2

[Lauf zu f2e2725](https://github.com/Lulus792/SecondBrain/actions/runs/37534667024)
besteht mit allen 18 Jobs: 21 Desktoptests je Debug/Release auf Windows x64,
macOS ARM64 und Linux x64 sowie drei entpackte Pakete. Die Teilfehlerbehandlung
besteht auch im Produktions-CLI-Prozesstest auf den nativen CI-Systemen. Lokal
besteht das entpackte Intel-Paket mit 126 Desktop-, 105 Tastatur- und 75 Sicherungs-
UI-Aussagen sowie Einstellungsprozessen und CLI-Sicherung. Archiv-SHA-256:
`9989fc424de596e021504e0ac71132231d20347d4e923cc68ab82e81f38a8c1d`. Menschliche assistive Bedienung und übrige Release-Abnahmen bleiben offen.


## Abnahme von 0.7.3

[Lauf zu 7331eca](https://github.com/Lulus792/SecondBrain/actions/runs/37536230101)
besteht mit allen 18 Jobs: 22 Desktoptests je Debug/Release auf Windows x64,
macOS ARM64 und Linux x64 sowie drei entpackte Pakete. Lokal besteht das entpackte
Intel-Paket mit 126 Desktop-, 105 Tastatur- und 75 Sicherungs-UI-Aussagen sowie
Einstellungsprozessen und CLI-Sicherung. Archiv-SHA-256:
`ce836ac9efc670a6a515539197387179b1a47c0df12183306de3f8f28e88f404`. Die vollständige Release-Arbeit bleibt offen.

## Vier Paketarchitekturen und erster Release, 7. Oktober 2026

Zu d109b8a/v0.7.3 besteht
[Lauf37537383565](https://github.com/Lulus792/SecondBrain/actions/runs/37537383565)
mit 22 Jobs: Version, 20 Plattformjobs und Veröffentlichung. Acht Desktopjobs
führen je 23 Tests in Debug/Release auf Windows x64, Linux x64, macOS ARM64
und Intel aus. Vier Releasejobs prüfen die entpackten Pakete einschließlich
Desktop, Tastatur, Sicherung, Einstellungen und CLI. Die öffentliche
[Vorabversion](https://github.com/Lulus792/SecondBrain/releases/tag/v0.7.3)
enthält diese vier Archive und SHA256SUMS; die Pipeline prüft Uploadbytes erneut.
Das belegt CI-Runner und Paketabläufe, keine vollständige reale Geräte- oder
Screenreader-Abnahme. 0.8.0 bleibt ein späterer, gesondert zu prüfender Stand.

## Textanbindung 0.9.0

[Lauf37543159232](https://github.com/Lulus792/SecondBrain/actions/runs/37543159232)
zu 3161f08 besteht mit 20 Jobs: 26 Desktoptests je Debug/Release auf Windows
x64, Linux x64, macOS ARM64 und Intel sowie vier entpackte Pakete. Die
Textprüfungen umfassen Verbindung, Fallbacks und Schriftgrößen im beschriebenen
Umfang. Gemischte Schreibrichtungen und vollständige assistive Abnahme bleiben
offen. Dieser Lauf ist kein Nachweis für die spätere Dokumentstruktur 0.9.1.

## Linux-Ebenen und Fehlerszenarien 0.9.2

Im [Lauf zu b69223c](https://github.com/Lulus792/SecondBrain/actions/runs/37548834947)
bestehen inzwischen beide nativen Linux-Desktopjobs (Debug und Release). Sie
führen 27 Tests einschließlich AT-SPI-Dokumenttext/-Ebenen, nativer Abschnitts-
anfragen und echter Prozess-Kills aus. Der Releasejob bestätigt zudem das
entpackte Linux-Paket. Die gehashte UI-Ebenenkorrektur ist damit auf Linux
nachgeprüft. Windows Release und macOS ARM64 Debug/Release sowie Intel Release
bestehen ebenfalls; weitere Jobs waren beim Eintrag noch nicht abgeschlossen.

Dieser Lauf ist insgesamt noch keine erfolgreiche Gesamtabnahme: sechs
Python-Jobs scheitern, weil die neuen Prozessskripte beim Import ausgeführt
wurden. Die Korrektur 1358794 trennt Import und Ausführung. Neun lokale
Generatorprüfungen und beide gezielten Prozess-/Volume-Nachprüfungen bestehen.
[Wiederholung](https://github.com/Lulus792/SecondBrain/actions/runs/37549092934)
ist separat abzunehmen. Keine Signierungs-, Geräte- oder menschliche
Screenreader-Abnahme aus diesen CI-Ergebnissen abgeleitet.

Der [Wiederholungslauf zu 1358794](https://github.com/Lulus792/SecondBrain/actions/runs/37549092934)
besteht inzwischen mit allen 20 Jobs und vier entpackten Paketen. Acht native
Desktopjobs prüfen Debug/Release auf Windows x64, Linux x64, macOS ARM64 und
Intel: 27 Tests auf Windows/Linux, 28 auf macOS (zusätzlich echtes Fehler-Volume).
Sechs C17-Kernjobs und sechs Python-Jobs bestehen ebenfalls. Damit sind die
Linux-Ebenenkorrektur und die importseitige Testkorrektur auf den zugesagten
CI-Systemen abgenommen. Dieser Nachweis betrifft 0.9.2; 0.9.3 folgt separat.

## Lizenzansicht 0.9.3

[Lauf37550880880 zu 3f3d3a0](https://github.com/Lulus792/SecondBrain/actions/runs/37550880880)
besteht mit allen 20 Jobs und vier entpackten Paketen. Acht Desktopjobs prüfen
Debug/Release auf Windows x64, Linux x64, macOS ARM64 und Intel: 28 Tests auf
Windows/Linux und 29 auf macOS. Die Ressourcenprüfung liest alle 17 Lizenztexte;
der Tastaturweg prüft erste, lange und letzte Auswahl, Kopieren, Scrollen und
Entwurfsschutz. Kern- und Python-Jobs bestehen ebenfalls. Dieser Lauf betrifft
0.9.3; die neuen Markdown-Blockregeln in 0.9.4 benötigen einen eigenen Lauf.

## Dokumentblockregeln 0.9.4

[Lauf37552977212 zu ee85750](https://github.com/Lulus792/SecondBrain/actions/runs/37552977212)
besteht in 19 von 20 Jobs. Windows Release, Linux Debug/Release und macOS
ARM64/Intel Debug/Release sowie vier entpackte Pakete bestehen. Die acht
Desktopjobs umfassen 29 Tests auf Windows/Linux und 30 auf macOS.
Windows Debug scheitert ausschließlich am 300-Sekunden-Timeout des Tastaturtests;
die übrigen 28 Tests einschließlich der neuen Blockrollen bestehen dort.
[Originalannotation](https://github.com/Lulus792/SecondBrain/actions/runs/37552977212/job/112572574929).
Der gesamte Lauf gilt daher nicht als bestanden.

0.9.5 zeichnet im Tastaturprüfwerkzeug den vollständigen Zustand nach Key-up
und verwirft nur die Zwischen-Zeichenbefehle von Key-down. Beide Phasen
berechnen weiterhin Eingabe, Layout, Animation, Modell und native Snapshots.
Die normalen App-Frames und aufgenommenen Prüfbilder werden weiterhin vollständig
gezeichnet. Die neue Windows-Debug-Abnahme steht aus; der Timeout wird nicht
allein durch die lokale Verbesserung als behoben bezeichnet.

Die [0.9.5-Abnahme zu 37dba58](https://github.com/Lulus792/SecondBrain/actions/runs/37556151012)
besteht inzwischen mit allen 20 Jobs einschließlich Windows Debug und vier
entpackten Paketen. Dieser Nachweis betrifft die geprüfte Version 0.9.5;
die unveröffentlichte Tabellenarbeit 0.9.6 ist noch nicht abgenommen.


## Entwicklungsschritt 0.9.6: lokale Tabellenabnahme

7. Oktober, Intel-Mac/macOS 14.6.1: 18 Debug-Kernprüfungen, anschließend drei
Markdown-Nachprüfungen und Tabellen-ASan/UBSan bestehen. Erster vollständiger
Release-Lauf: 32 Tests in 470,15 s. Abschließende sechs passende Tests nach den
Darstellungskorrekturen: 34,86 s, einschließlich 256 nativer Assertions.
41 Graph-Assertions bestehen separat. Ausgeführte macOS-Provideraufrufe prüfen
4×3 Tabellenrollen/Zellwerte und eine Scrollanfrage; Rasteransichten bei normaler
und 200-%-Schrift wurden betrachtet. [Umfang und Grenzen](TABELLEN.md).

Das entpackte Intel-Paket 0.9.6 besteht mit 126 Desktop-, 142 Tastatur- und
75 Sicherungsassertions, zwei Einstellungs-Neustartprozessen und dem produktiven
Sicherungswerkzeug. Archiv-SHA-256:
`4d35909f54230806131886602d7872885270d1a595a4734e4ebb936262ff64f7`.
Die geprüfte lokale Kopie liegt unter dist/SecondBrain. Der
[20-Job-Lauf zu 648a696](https://github.com/Lulus792/SecondBrain/actions/runs/37593650781)
läuft noch. Native Windows-/Linux-/ARM64-Ergebnisse werden anhand des tatsächlich
abgeschlossenen Laufs nachgetragen. Die Windows-/Linux-Clientprüfungen für Tabellenkinder sind bisher
nur vorbereitet; fehlende UIA-/AT-SPI-Matrixschnittstellen bleiben offen.


## Nachprüfung und laufender Schritt 0.9.7

Der 0.9.6-Lauf zu 648a696 besitzt einen bestätigten Fehler: Windows Debug,
native-accessibility, Timeout120s; übrige 30 UI-Tests dieses Jobs bestehen.
[Öffentlicher Fehlernachweis](https://github.com/Lulus792/SecondBrain/actions/runs/37593650781/job/112700888022).
Er darf nicht als vollständige Plattformabnahme gelten.

0.9.7 verfeinert die schmale Tabellenansicht und verringert ausschließlich die
Rasterarbeit im nativen Prüfprogramm. Fünf passende lokale Tests und die
abschließende native Prüfung mit 270 Assertions bestehen. Der frühere Renderer
scheitert an der Kopfzeilenregression; Rasterbilder mit 200 % wurden betrachtet.
Das entpackte Intel-Paket 0.9.7 besteht inzwischen mit 126 Desktop-, 142
Tastatur- und 75 Sicherungsassertions, zwei Einstellungs-Neustartprozessen sowie
dem Sicherungswerkzeug. SHA-256: `81da4693a2efdb816f782c8f8dd4710fb75643ec62bd477664108bc6da5e31de`.
Die Umsetzung ist als 2edb2bf gepusht. Der
[native Lauf 37596496154](https://github.com/Lulus792/SecondBrain/actions/runs/37596496154)
läuft noch; insbesondere Windows Debug ist erst anhand seines neuen tatsächlichen
Ergebnisses abgenommen.


Der 0.9.6-Lauf 37593650781 ist inzwischen abgeschlossen: 19 von 20 Jobs bestehen,
einschließlich aller vier entpackten Release-Pakete. Einziger Fehler bleibt
Windows Debug / native-accessibility / Timeout 120s. Die erfolgreichen Pakete
belegen ihren konkreten Release-Ablauf, ersetzen aber keinen grünen Debug-Job.


Nachprüfung zu 2edb2bf am 7. Oktober: Der neue native Lauf 37596496154 besteht
inzwischen für Windows und Linux in Debug und Release, einschließlich der
nativen Prüfung und der jeweiligen entpackten Release-Pakete. Windows Debug
ist damit nach dem vorherigen nativen Timeout wieder vollständig erfolgreich.
Die noch laufenden/ausstehenden Mac-Jobs werden separat nach Abschluss bewertet.


## Graphemarbeit 0.9.8 und abgeschlossene Vorversion

[0.9.7 zu 2edb2bf](https://github.com/Lulus792/SecondBrain/actions/runs/37596496154)
besteht mit allen 20 Jobs und vier entpackten Paketen für Windows x64, Linux x64,
macOS ARM64 und Intel einschließlich des zuvor fehlgeschlagenen Windows Debug.

0.9.8 besteht lokal mit 19 Debug-Kernprüfungen, allen 853 offiziellen Graphemfällen,
ASan/UBSan für den Segmentierer,33 ersten Release-Tests und sechs abschließenden
passenden UI-/Kernprüfungen. 273 native Assertions umfassen eine tatsächliche macOS-
Teilmarkierung, die auf den vollständigen Akzent erweitert wird; 111 Editor-
Assertions prüfen Zeicheneinheiten und vollständige Textläufe. Ein früherer
Intel-Paketlauf scheitert an fremdem Zwischenablageinhalt im Pfadfixture; der
korrigierte Sicherungsablauf besteht lokal, das neu erzeugte Paket wird geprüft.
Neue native Windows-/Linux-/ARM64-Nachweise werden nach tatsächlichem Abschluss
übernommen. [Genauer Umfang und Grenzen](GRAPHEME.md).


Das abschließende, neu erzeugte und entpackte Intel-Paket 0.9.8 besteht mit
126 Desktop-, 142 Tastatur- und 75 Sicherungsassertions, zwei Einstellungs-Neustart-
prozessen und dem produktiven Sicherungswerkzeug. Der Unicode-Lizenztext ist
im Paket geprüft. Archiv-SHA-256: `ecef5c9c0027629cdfb7287879b5749f42b919498f35010e00b6155269bb93c0`.
Log: build/grapheme-package-verified-check.log. dist/SecondBrain enthält dieses
geprüfte Entwicklungspaket, lokale Buildkennung 83ce90d6ee9b-dirty (vor Commit
gepackt). Der Quellstand 605527e ist gepusht; die
[native Abnahme 37604300744](https://github.com/Lulus792/SecondBrain/actions/runs/37604300744)
läuft noch. Die frühere fehlerhafte Paketprüfung wird dadurch nicht nachträglich
als bestanden geführt. Emoji-Glyphen, komplexe Geometrie und menschliche
assistive Bedienung bleiben offen.


## Lokale Schriftlauf-Nachprüfung 0.9.9

33 erste Intel/macOS-Release-Tests bestehen (620,10 s). Nach abschließender
Raster-Fehlerbehandlung und Alpha-Korrektur bestehen drei gezielte Prüfungen
mit 43 Text-, 111 Editor- und 58 Lizenzassertions. Eigene C-Text-/Graphem-Bereiche
bestehen unter ASan/UBSan; vorherige reine Fallback-Anbindung scheitert an der
neuen verbundenen Emoji-Prüfung. [Details und Grenzen](EMOJI.md).
Das neue entpackte Paket und native Windows-/Linux-/ARM64-Abnahmen sind noch
offen; die vorherige dist-Kopie bleibt bis zur bestandenen Paketprüfung erhalten.


Das entpackte Intel-Paket 0.9.9 besteht mit 126 Desktop-, 142 Tastatur- und
75 Sicherungsassertions, zwei Einstellungs-Neustartprozessen und dem produktiven
Sicherungswerkzeug. Schrift und originale Emoji-Lizenz sind im Paket geprüft.
Archiv-SHA-256: `83c17797c732e45df03eda7e482d4462ec760309cf815ad12eb4b52817b412f1`.
Log: build/emoji-package-check.log. dist/SecondBrain enthält dieses geprüfte
Release-Konfigurationspaket mit Buildkennung cf5678a1a177; die frühere lokale
Kopie ist unter build/emoji-previous-dist-20261007-123656 erhalten. Das eigene
Projektgedächtnis wurde mit diesem Paket geladen und als Raster betrachtet.

Die [erste native 0.9.9-Abnahme](https://github.com/Lulus792/SecondBrain/actions/runs/37608033806)
findet eine zu enge Familienbreiten-Assertion auf Linux und Windows: 17 statt
16 Rasterpixel. Die übrigen 31 Release-Tests dieser Systeme bestehen; der
jeweilige Paketlauf wird wegen des Texttestfehlers nicht ausgeführt. SDL_ttf
berechnet Rastergrenzen einschließlich Glyphenüberhängen (pinned SDL_ttf.c,
TTF_Size_Internal). Der Test lässt für diese unterschiedliche Glyphe nun zwei
logische Pixel Abweichung zu und verlangt zusätzlich eine Breite unter der
Hälfte der vier separat gemessenen Familienfiguren. Die zerlegte Ausgabe bleibt
so erkannt. Lokale Textnachprüfung besteht mit 46 Assertions (1,19 s), eigene
ASan/UBSan-Nachprüfung ebenfalls; frühere Fallback-Fassung scheitert weiter.
Logs: build/emoji-rounding-check.log, emoji-sanitize/rounding-result.log und
emoji-regression/rounding-result.log. Neuer nativer Lauf folgt nach Push.
Diese Testkorrektur verändert das bereits gepackte Anwendungsprogramm nicht.


## Lokale Nachprüfung 0.9.10

Abschnittstrennungen, native macOS-Separatorrolle, skalierte Reader-Innenabstände
und Hilfe-Punkt sind implementiert. Abschließender vollständiger Intel/macOS-
Release-Lauf besteht mit 33 Tests (195,44 s), einschließlich realer Kontrastpixel
und beidseitigem Abstand zum inneren Fokusring. Blockerkennung/Test besteht in
eigener ASan/UBSan-Instrumentierung; 100/200-Prozent-Raster sind betrachtet.
Native UIA-/AT-SPI-Separatorabfragen sind vorbereitet, neue Ausführung folgt
nach Push. Paketprüfung folgt separat. [Details](TRENNLINIEN.md).

0.9.9 zu 099de24 besteht inzwischen unter Windows und Linux einschließlich aller
Debug-/Release- und Paketwege; Mac-Jobs fehlen noch. Der zuvor fehlgeschlagene
Windows-Debug-Kernlauf hat keine festgestellte Ursache, auch wenn der neue Lauf
besteht. Vollständige Plattform-/Geräteabnahme daraus nicht ableiten.


Die [erste native 0.9.10-Abnahme zu 92b293a](https://github.com/Lulus792/SecondBrain/actions/runs/37612203745)
scheitert in Windows/Linux Release am realen Kontrastpixel; Rollen und Abstände
bestehen bis dahin. Lokal mit einem tatsächlichen SDL-Fenster ohne das High-
Pixel-Density-Flag reproduziert: Pixeldichte 1, RGB 128/128/128 statt 0/0/0.
Die dünne geglättete Linie hat dort keinen deckenden Kern. Im Kontrastmodus
verwendet die Produktion jetzt mindestens zwei integrale logische Stricheinheiten
und mindestens drei Rasterpixel; der übrige Modus behält seine dezente Linie.
Native Rechtecke verwenden dieselbe tatsächliche Stärke. Das Farbkriterium bleibt
streng; es wird nicht auf Grau gelockert. Nachprüfung besteht mit 352 Assertions
bei realer Pixeldichte 2 und in der lokalen 1x-Fensterfixture, jeweils bei 100/200
Prozent Schriftgröße. Beide liefern RGB 0/0/0. Die Fixture verändert nur das
Fensterflag, keine vorgegebenen Rückgabewerte. Das ist keine Windows-/Linux-
Bedienabnahme. Logs: build/rules-density-proof/{before,after}.log und
rules-density-final-2x-check.log. Neues Paket und neue native Abnahme folgen.


Das abschließende, korrigierte Intel-Paket 0.9.10 zu f3a758e besteht mit
126 Desktop-, 142 Tastatur- und 75 Sicherungsassertions, zwei Einstellungs-
Neustartprozessen und dem produktiven Sicherungswerkzeug. Archiv-SHA-256:
`110062f29d98ca0f9150e39cc4985cde03d3e8c34dcaee436588a67ff9893bbb`. Log: build/rules-verified-package-check.log.
dist/SecondBrain enthält dieses Paket mit Buildkennung f3a758eb61d9. Die frühere lokale Kopie
ist unter /Users/lulus/Projects/SecondBrain/build/rules-verified-previous-dist-20261007-132843 erhalten. Das eigene Projektgedächtnis wurde mit dem Paket
geladen und als Raster betrachtet. [Neue native Abnahme](https://github.com/Lulus792/SecondBrain/actions/runs/37613957814)
läuft noch; insbesondere die früher fehlerhaften Kontrastfälle werden erst anhand
ihrer tatsächlich abgeschlossenen neuen Ergebnisse abgenommen.


Die [abschließende native 0.9.10-Abnahme zu f3a758e](https://github.com/Lulus792/SecondBrain/actions/runs/37613957814)
ist am 7. Oktober vollständig erfolgreich: 20 Jobs einschließlich Windows/Linux
Debug/Release, Intel-/ARM64-macOS und tatsächlich entpackter Release-Pakete.
Die früher fehlerhaften Kontrastfälle bestehen in den neuen Desktopläufen.
Dies bestätigt den automatisierten Umfang; keine menschliche assistive oder
Geräteabnahme daraus ableiten.

0.9.11 ist lokal auf Intel/macOS 14.6.1 geprüft: 34 erste Release-Tests und sieben
abschließende betroffene Nachprüfungen bestehen, dazu eigene ASan/UBSan und neun
Python-Strukturtests. Neue native Plattform-/Paketabnahme folgt nach Push.
Einzelheiten und Grenzen im [Umsetzungsstand](STATUS.md#gemeinsame-hervorhebungen-in-0911).


## Geprüftes Intel-Paket 0.9.11

Das entpackte Release-Paket zu 54aa29f besteht mit 126 Desktop-, 142 Tastatur-
und 75 Sicherungsassertions. Zwei tatsächliche Neustartprozesse erhalten ihre
isolierten Einstellungen; die produktive CLI sichert, prüft und stellt mit
Unicode-Pfaden wieder her. App/CLI und Bundle melden 0.9.11, Build 54aa29fd7bc8.
Archiv-SHA-256: `0e7957600004802232be96e15f0968febecf4e4a4d24e10034c3a55f9859b828`.
Log: build/emphasis-package-check.log. dist/SecondBrain enthält jetzt das
geprüfte Paket. Vorherige lokale App erhalten unter /Users/lulus/Projects/SecondBrain/build/emphasis-previous-dist-20261007-143541.
Das eigene Projektgedächtnis wurde mit dem Paket geladen und als Raster betrachtet.

Die [neue native Abnahme](https://github.com/Lulus792/SecondBrain/actions/runs/37621658743)
läuft. Zum dokumentierten Zwischenstand bestehen alle sechs C-Kern- und sechs
Python-Jobs; die acht Desktopjobs laufen oder warten noch. Neue automatische
Desktop-/Paketabnahmen erst nach tatsächlichem Abschluss übernehmen. Menschliche
assistive Bedienung, native Stilattribute und weitere Release-Aufgaben bleiben offen.


Die [native 0.9.11-Abnahme zu 54aa29f](https://github.com/Lulus792/SecondBrain/actions/runs/37621658743)
ist vollständig erfolgreich: 20 Jobs auf Windows, Linux, ARM64- und Intel-macOS,
jeweils einschließlich des vorgesehenen Debug-/Release-Umfangs und tatsächlich
entpackter Release-Pakete. Alle sechs C-Kern-, sechs Python- und acht Desktopjobs
bestehen. Die gemeinsamen Hervorhebungen sind damit im automatisierten Umfang
abgenommen; menschliche assistive und Geräteabnahmen bleiben offen.


## Geprüftes Intel-Paket 0.9.12

Das entpackte Paket zu 4c4a510 besteht mit 126 Desktop-, 142 Tastatur- und
75 Sicherungsassertions. Zwei Neustartprozesse erhalten ihre isolierten
Einstellungen; die produktive CLI sichert, prüft und stellt mit Unicode-Pfaden
wieder her. Bundle, App und CLI melden 0.9.12, Build 4c4a51054334. Archiv-SHA-256:
`2db376cf28c1ccc3ec4cb3dbcd2d0a9c487f2e826f729742f9098694ed15b6c1`.
Log: build/native-styles-package-check.log. dist/SecondBrain enthält dieses
geprüfte Paket. Vorherige lokale App bleibt unter /Users/lulus/Projects/SecondBrain/build/native-styles-previous-dist-20261007-150228 erhalten.
Eigenes Projektgedächtnis mit dem neuen Paket geladen und Raster betrachtet.

Die [neue native 0.9.12-Abnahme](https://github.com/Lulus792/SecondBrain/actions/runs/37624868119)
läuft. Beim dokumentierten Zwischenstand sind zwölf Jobs erfolgreich und acht
in Arbeit; insbesondere neue UIA-/AT-SPI-Stilabfragen werden erst nach ihrem
tatsächlichen Abschluss abgenommen. Menschliche assistive Bedienung und weitere
Release-Aufgaben bleiben offen; Version 1.0 ist nicht freigegeben.


## Tatsächliche Windows-/Linux-Abnahme zu 9c38604

Der [Lauf 37631991014](https://github.com/Lulus792/SecondBrain/actions/runs/37631991014)
besteht unter Windows und Linux in Debug und Release, einschließlich der nativen
Stilabfragen. Windows kompiliert die neue UI-DLL aus den festgelegten Quellen;
die neun FindText-Fälle bestehen mit ersten/letzten Treffern, Großschreibung,
fehlenden Treffern, Emoji-Indizes, Anfang und begrenzten Suchbereichen.
Beide tatsächlich entpackten Release-Pakete bestehen ihre Abläufe. Insgesamt
bestehen alle zwölf Windows-/Linux-Jobs;
neue Mac-Jobs sind beim dokumentierten Zwischenstand noch in der Warteschlange.

Damit sind FindText und die vorher fehlgeschlagenen Windows-Stilabfragen im
automatisierten Umfang nachgeprüft. Keine menschliche NVDA-/Orca-/VoiceOver-
Abnahme daraus ableiten. MinGW, genaue Textgeometrie und weitere Release-
Aufgaben bleiben offen. Die strenge Quellenprüfung bleibt erhalten; der
endgültige Patchhash lautet 453fcaaa4faf52c87cc8f15fc50d4710540e35bc181bd6ac3daf28613ed7874a.


## Abschließendes lokales Intel-Paket 0.9.13

Das aus dem korrigierten Quellstand gepackte Intel/macOS-Paket besteht mit
126 Desktop-, 142 Tastatur- und 75 Sicherungsassertions, zwei Einstellungs-
Neustartprozessen sowie der produktiven Sicherungs-/Prüf-/Wiederherstellungs-CLI.
App und CLI melden 0.9.13, Build cade9ea34b95. Archiv-SHA-256:
`1ac29c6ba03c8df5880f254309a2fed82c0cedd8ad1256f199805e12d6e99cde`.
Log: build/windows-findtext-verified-package-check.log. dist/SecondBrain enthält
dieses geprüfte Paket; vorherige App unter /Users/lulus/Projects/SecondBrain/build/windows-findtext-previous-dist-20261007-160855 erhalten. Eigenes
Projektgedächtnis geladen und Raster betrachtet. Die neue Mac-CI bleibt separat
zu übernehmen; die beschriebenen Windows-/Linux-Abnahmen zu 9c38604 bestehen.
1.0, menschliche assistive Bedienung und verbleibende Release-Arbeiten bleiben offen.


## Paket- und Plattformnachprüfung 0.9.14

Am 7. Oktober 2026 besteht das neue lokale Intel/macOS-Paket zu ddd2186:
126 Desktop-, 142 Tastatur- und 75 Sicherungsassertions, zwei isolierte
Einstellungs-Neustartprozesse sowie die produktive Sicherungs-/Prüf-/
Wiederherstellungs-CLI. App und CLI melden 0.9.14, Build ddd2186d2ab9.
Log: build/entities-package-check.log. Archiv-SHA-256:
`0d1e05c6593b972e8643a5cf3035d27b8be141341c77f3ece55064a8e431b0ce`.
Neue Math-Schrift und Original-Lizenztexte sind im Archiv anhand ihrer
festgelegten Prüfsummen nachgeprüft, einschließlich der Kopien im App-Bundle.

Die Arbeitskopie unter dist/SecondBrain enthält dieses Paket. Vorherige App
bleibt unter /Users/lulus/Projects/SecondBrain/build/entities-previous-dist-20261007-165530 erhalten. Das eigene Projektgedächtnis wurde mit dem
installierten Paket geladen; build/entities-own-brain.bmp und
dist/SecondBrain/preview.png wurden erzeugt, die PNG-Darstellung betrachtet.

[Lauf 37639562794](https://github.com/Lulus792/SecondBrain/actions/runs/37639562794)
zu ddd2186 besteht in allen zwölf Windows-/Linux-Jobs: Python, C17 und Desktop
je Debug/Release, einschließlich tatsächlich entpackter Release-Pakete. Die
neuen macOS-CI-Jobs sind beim dokumentierten Zwischenstand noch nicht vollständig
abgeschlossen. Der lokale Intel-Test ist deshalb der hier abgeschlossene
macOS-Paketnachweis; eine neue ARM64-Gesamtabnahme bleibt offen.

SBUI-048/049 sind im genannten lokalen sowie automatisierten Windows-/Linux-
Umfang nachgeprüft. Bidi, Textgeometrie, IME, vollständige Markdown-Container,
native Tabellenmatrixschnittstellen, menschliche assistive Bedienung und die
übrigen Release-Aufgaben bleiben offen. Der Gesamtauftrag bleibt aktiv;
die Versionsnummer 1.0 wurde nicht gesetzt.


## Abschließendes lokales Paket 0.9.16 und erfolgreicher Push

Das Intel/macOS-Paket zu b19b476 besteht mit 126 Desktop-, 142 Tastatur- und
75 Sicherungsassertions, zwei isolierten Einstellungs-Neustartprozessen sowie
der produktiven Sicherungs-/Prüf-/Wiederherstellungs-CLI. App und CLI melden
0.9.16, Build b19b47624512. Archiv-SHA-256:
`99f3bd806a8321cf408968f223295c8d313c4247488577bf7caae7ffd7905963`.
Log: build/references-package-check.log. Die vorhandene Unicode-Lizenz ist
in Paket und Bundle enthalten. dist/SecondBrain enthält dieses geprüfte Paket;
vorherige App unter /Users/lulus/Projects/SecondBrain/build/references-previous-dist-20261007-174739 erhalten. Das eigene Gedächtnis wurde geladen
und die erzeugte Vorschau dist/SecondBrain/preview.png betrachtet.

Der normale Push a57bdfc..b19b476 ist erfolgreich abgeschlossen. Die ausstehenden
0.9.15-Commits und die neue 0.9.16-Implementierung sind damit veröffentlicht.
Die vorherigen serverseitigen Abweisungen bleiben historische Zwischenstände;
keine Historie wurde umgeschrieben.

[Lauf 37646348381](https://github.com/Lulus792/SecondBrain/actions/runs/37646348381)
prüft b19b476. Beim dokumentierten Zwischenstand bestehen die vier C17- und
vier Python-Jobs für Windows/Linux. Die vier Desktop-Jobs laufen noch, die
macOS-CI ist noch in der Warteschlange. Neue abgeschlossene Desktop-/Paket-
oder ARM64-Abnahme daraus noch nicht ableiten. Nachfolgende Ergebnisse
werden anhand des tatsächlichen Laufs übernommen.

SBUI-052/053 im beschriebenen lokalen Umfang nachgeprüft; vollständige
Container und ihre Definitionen, Bidi/Textgeometrie/IME, native Tabellenmatrix,
menschliche assistive Bedienung und übrige Release-Aufgaben bleiben offen.
Der vollständige Auftrag bleibt aktiv; keine Versionsnummer 1.0 gesetzt.


## 0.9.16: abgeschlossene Windows-/Linux-Nachprüfung

[Lauf 37646348381](https://github.com/Lulus792/SecondBrain/actions/runs/37646348381)
zu b19b476 besteht in allen zwölf Windows-/Linux-Jobs: Python, C17 und Desktop
in Debug/Release. Beide Release-Jobs bestehen ihre tatsächlich entpackten
Paketabläufe. Damit sind Referenzlinks, Unicode-Namen und Scroll-/Fokusabstand
im automatisierten Umfang auf diesen beiden Plattformen nachgeprüft. Die
neuen macOS-CI-Jobs sind beim dokumentierten Zwischenstand noch in der Warteschlange.
Menschliche assistive Bedienung und weitere Release-Arbeiten bleiben offen.


## 0.9.19: Ergebnis der neuen Desktop-Prüfung

[Lauf 37680520037](https://github.com/Lulus792/SecondBrain/actions/runs/37680520037)
zu 9ab187a besteht in macOS ARM64/Intel jeweils Debug/Release einschließlich
Release-Paketen sowie Windows-Debug. Windows-Release scheitert im UI-Testschritt;
beide Linux-Jobs sind abgebrochen. Der öffentliche Job meldet kein verfügbares
Testlog. Ohne die anmeldepflichtigen Details bleibt die Ursache ungeklärt.
Ab 0.9.20 schreibt der native Teststarter vor dem CMake-Aufruf ein Eintragslog;
dieser Diagnoseschritt ist lokal mit echten CMake-/CTest-Prozessen geprüft.
Er ersetzt weder die ausstehende Fehleranalyse noch eine neue Plattformabnahme.

## 0.9.21: vollständige neue CI-Abnahme

[Lauf 37695359274](https://github.com/Lulus792/SecondBrain/actions/runs/37695359274)
zu 82c2970 besteht am 8. Oktober in allen 20 Jobs: acht Desktop-Jobs auf
Windows, Linux und macOS ARM64/Intel, sechs C17-Jobs und sechs Python-Jobs.
Alle vier Release-Jobs prüfen ihre tatsächlich entpackten Pakete erfolgreich.
Damit sind kanonische Windows-TEMP-Pfade und die korrigierte Synchronisation
der sechs Produktions-Abbruch-/Wiederanlaufpunkte im ausgeführten Umfang
nachgeprüft. Windows-Testfixture verwendet einen echten exklusiven Handle;
die zusätzliche Unix-Aliasprobe benötigt unter Windows keine Symlink-Rechte.

Die Cargo-Compileraufzeichnung bestätigt in Windows-/Linux-Release jeweils
1.98.1, Commit 48a229ceaefd4985c50990b14116b6d856af0985, LLVM 22.1.8,
mit den entsprechenden nativen Hosts. Neue 0.9.22-Lizenzdaten benötigen ihren
eigenen Plattform-/Paketnachweis. Menschliche assistive, Geräte-, Volumen- und
weitere Release-Abnahmen bleiben offen.


## 0.9.22: abgeschlossene native Prüfung

[Lauf 37697739324](https://github.com/Lulus792/SecondBrain/actions/runs/37697739324)
zu 4e6f15c besteht in allen 20 Jobs: Windows/Linux/macOS ARM64/Intel Desktop
Debug/Release, C17 und Python. Alle vier Release-Pakete sind entpackt geprüft.
Die Root-/Registry-/Quellhinweise der Rust-Laufzeit und die 24 Lizenzressourcen
sind damit im automatisierten Umfang nachgeprüft. Die danach ergänzte native
Importprüfung erhält ihren eigenen folgenden CI-Nachweis.

## 0.9.23: Runtime-Prüfung auf allen Paketarchitekturen

[CI 37704298271](https://github.com/Lulus792/SecondBrain/actions/runs/37704298271)
zu 7f9acb8 besteht am 8. Oktober in allen 20 Jobs. Windows/Linux/macOS ARM64/
Intel bestehen jeweils Desktop-Debug/Release; alle vier entpackten Release-
Pakete bestehen mit der neuen Importprüfung. Die statische MSVC-CRT-Konfiguration
besteht auch unter Windows-Release. C17- und Python-Matrix bestehen. Frische
Nutzerrechner, Mindestversionen und menschliche Abnahmen bleiben offen.
