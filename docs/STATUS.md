# Umsetzungsstand von SecondBrain

Stand: 7. Oktober 2026. Die Vorabversion 0.7.3 ist als dauerhafter GitHub Release
veröffentlicht. 0.8.0 mit Versionsangaben ist auf allen vier Paketarchitekturen abgenommen.
Der aktuelle Entwicklungsschritt 0.9.0 ergänzt geformten Text und Ersatzschriften.
Die Abschnitte nennen die tatsächlich ausgeführten Abnahmen und deren Grenzen.

## Abgeschlossene Grundlagen

- Apple UI-Grundlagen wurden vor dem Entwurf eingesehen und unter UI_RECHERCHE.md
  mit Originalquellen dokumentiert.
- UI_ENTWURF.md beschreibt Projektwahl, Dokumentliste, Lesen, Bearbeiten, Suche,
  Quellenansicht, Kontext und den Schutz ungespeicherter Änderungen.
- ARCHITEKTUR.md trennt den eigenen C17-Kern von UI-Abhängigkeiten.

## Implementierter C Kern

Der C-Kern kann Projekte aus den bestehenden Vorlagen anlegen, vorhandene
Projektgedächtnisse erkennen, Dokumente auflisten, laden, speichern und
archivieren. Er durchsucht Titel und Inhalte und stellt die Kerninformationen
als KI-Kontext zusammen. Externe Änderungen werden vor einem Speichervorgang
anhand des gelesenen Inhalts erkannt. Vorhandene Zielordner werden nicht ersetzt.

Windows-Dateizugriff verwendet UTF-16-Systemaufrufe; nach außen bleiben die
Dateien und Pfade UTF-8. macOS und Linux verwenden die eigene POSIX-Anbindung.
Der Kern linkt ausschließlich gegen Standard- und Betriebssystembibliotheken.
Die Vorlagen werden beim CMake-Konfigurieren in den Kern eingebettet.

Ein natives Kommandozeilenwerkzeug dient der Entwicklung und der Dateianbindung
an KI-Werkzeuge. Die normale Nutzung erfolgt über die eigene Oberfläche.

## Ausgeführte Prüfungen

Auf dem lokalen Intel-Mac mit macOS 14.6.1 und AppleClang 16 bestehen Build und
Kernablauf. Ein zusätzlicher Lauf mit AddressSanitizer und UndefinedBehaviorSanitizer
besteht ebenfalls. Die Kernprüfung verwendet Projektauftrag, Entscheidungen und
Wissensnotizen aus einem neutralen Testprojekt.

Die Tests prüfen echte Dateien: Erstellung, Wiederöffnung, UTF-8, Suchtreffer,
Speicherkonflikte, Archivierung, Kontext und gültige beziehungsweise fehlerhafte
JSON-Metadaten. Die C17-CI-Matrix ergänzt die bisherigen Python-Prüfungen um
Windows, macOS und Linux in Debug und Release.
Der [C17-Lauf zu 4c99bb3](https://github.com/Lulus792/SecondBrain/actions/runs/37435200479)
besteht auf allen drei Systemen in beiden Buildprofilen.

## Implementierte UI Grundlage

Die eigene Anwendungsschicht verbindet Projekt- und Dokumentwechsel mit einem
gemeinsamen Schutz ungespeicherter Inhalte. Speichern, Verwerfen und Abbrechen
sind für Wechsel, Archivierung und Beenden implementiert. Externe Textquellen
können schreibgeschützt geladen werden, während die eigene Bearbeitung erhalten
bleibt. Bei einem Speicherkonflikt kann eine eigene Fassung als neue Wissensnotiz
gesichert werden. Diese Zustandsabläufe sind anhand tatsächlicher Dateien geprüft;
ihre Bedienelemente sind in das Hauptfenster eingebunden.

SDL3 und Nuklear sind ausschließlich im UI-Ziel eingebunden. Die Grundlage
verarbeitet Texteingaben, stellt Schriften mit passender Pixeldichte dar und
bietet helle und dunkle Farben sowie vergrößerte Schrift. UI-Quellen, Versionen
und Lizenzen sind unter third_party dokumentiert.

Lokal bestehen Renderer- und Editorprüfungen, auch mit AddressSanitizer und
UndefinedBehaviorSanitizer. Sie prüfen Unicode-Paste, Auswahlersetzung,
Rückgängig/Wiederholen, lange Cursorpositionen, Kapazitätsfehler und mehrteilige
Texteingaben. Die tatsächliche Testdarstellung wurde betrachtet. Die UI-CI
prüft diese Grundlage zusätzlich mit nativen Fenstersystemen der drei Plattformen.
Der [UI-Lauf zu 2c1a911](https://github.com/Lulus792/SecondBrain/actions/runs/37437068287)
besteht mit erfolgreichen UI-Jobs auf Windows, macOS und Linux. Diese Bausteinprüfung
belegt die Bausteine; die Desktop-Prüfung wird unten separat beschrieben.

## Eigenes Projektgedächtnis

- [x] Ein eigenes Second Brain für dieses SecondBrain-Projekt erstellen und in
  der Anwendung verwenden. Es soll Projektziele, aktuellen Stand, Entscheidungen,
  Quellen und offene Aufgaben enthalten und dem Nutzer sowie der KI als
  gemeinsames Projektgedächtnis dienen.
  [Die Instanz](../brains/secondbrain/START.md) wurde mit dem C-Kern angelegt,
  befüllt, durchsucht und in der eigenen App geladen und betrachtet.
  Alle lokalen Quellenverweise und der relative Repository-Verweis wurden geprüft.

## Desktop und Pakete

Die Desktop-Oberfläche ist implementiert: Projekte und Notizen anlegen,
Markdown lesen und bearbeiten, speichern, suchen, archivieren, Quellen und
Ordner schreibgeschützt betrachten sowie KI-Kontext kopieren. Die Bedienprüfung
steuert dieselben Komponenten mit SDL-Maus-, Tastatur- und Zwischenablageereignissen.
Lokal bestehen alle fünf C-/UI-Prüfungen, auch mit AddressSanitizer und
UndefinedBehaviorSanitizer. Tatsächlich gerenderte Ansichten wurden
in groß/hell und klein/dunkel mit 150 Prozent Schriftgröße betrachtet.

Der [Desktop-Lauf zu 07ca223](https://github.com/Lulus792/SecondBrain/actions/runs/37443300206)
besteht auf Windows, macOS und Linux. Seine drei UI-Jobs führen alle vier
C-/UI-Prüfungen einschließlich des vollständigen Bedienablaufs aus.

CPack erzeugt Pakete mit statischem SDL, Schriften und Lizenzen. Lokal besteht
die entpackte macOS-App den Bedienablauf aus einem anderen Arbeitsordner mit
Leerzeichen und Umlauten. `otool -L` zeigt ausschließlich Systembibliotheken.
Der [Paketlauf zu aeb1d0d](https://github.com/Lulus792/SecondBrain/actions/runs/37443877273)
besteht mit sechs Desktop-Jobs (Debug und Release) und drei erfolgreichen Prüfungen
der entpackten Pakete auf Windows x64, macOS ARM64 und Linux x64.
Die Pakete stehen als Actions-Artefakte zum Download bereit.

Die [Abnahme zu 287ef48](https://github.com/Lulus792/SecondBrain/actions/runs/37445131476)
besteht mit allen 18 Jobs. Jeder der sechs Desktop-Jobs führt fünf Prüfungen aus;
die Release-Jobs prüfen zusätzlich die entpackten Pakete. Der Ablauf umfasst
Konfliktkopie, Archivierung, Speichern beim Beenden und den Import dieser eigenen
Projektinstanz. Lange Titel wurden in kleiner Darstellung mit großer Schrift geprüft.
Lokal liegt die Intel-macOS-App unter `dist/SecondBrain/secondbrain.app`.

Version 0.1 erfüllt den festgelegten lokalen Arbeitsablauf. Erweiterungen und
bekannte Grenzen stehen unter [Distribution](DISTRIBUTION.md) und im eigenen
[Fragenregister](../brains/secondbrain/QUESTIONS.md).
Nuklear besitzt hier keine Anbindung an native
Screenreader; vergrößerte Schrift und Tastaturbefehle ersetzen diesen fehlenden
Zugang nicht.

## Version 0.2: Lumen, Glaskarten und Tastatur

Die ausgewählte Sternkarte ist in C implementiert. Reale Projektdokumente bilden
kleine Lichtpunkte; interne Markdown-Verweise bilden Linien. Die Kamera reagiert
auf Ziehen, Verschieben und Zoom sowie Tastatur und sichtbare Schaltflächen.
Projektwahl, Suche, Dokumentliste und Lesekarte schweben über dem Raum.
Erstellen, Bearbeiten, Speichern, Quellen, Kontext, Konfliktkopie, Archivierung
und Schutzdialog verwenden die bisherigen fachlichen Modelle.

Eigene C-Materialberechnung bricht den Sternhintergrund unter gewölbten Flächen
und erzeugt Lichtkanten. Schrift wird separat dargestellt. Diese optische
Nachbildung verwendet keine native Apple-Liquid-Glass-Komponente. Die dunkle
Lumen-Palette ist der Standard; eine helle Variante und reduzierte Transparenz
sind verfügbar. Kameraänderungen erfolgen direkt ohne automatische Rotation.

Tab / Umschalt+Tab, sichtbare Fokusmarkierungen, Enter/Leertaste, F6, Pfeile und
Lesescroll ermöglichen reine Tastaturwege. Dialoge begrenzen den Fokus und
stellen ihn beim Schließen wieder her. Der Dokumenteditor hat einen dauerhaften
Undo-Zustand, der von Such- und Dialogfeldern getrennt ist. Beim Dokumentwechsel
wird er zurückgesetzt. [Bedienvertrag und Nachweise](UI_TASTATUR.md).

Lokal bestehen am 6. Oktober alle sieben C-/UI-Prüfungen in Release sowie mit
AddressSanitizer/UndefinedBehaviorSanitizer auf Intel macOS 14.6.1. Die tatsächlichen
Bedienprüfungen enthalten 87 Aussagen für Maus/Tastatur und 85 für reine Tastatur,
auch im kleinen Fenster mit 150 Prozent Schriftgröße. Die Graphprüfung umfasst
20 Aussagen, einschließlich relativer/UTF-8-Verweise und Code-/Bildausschlüssen.
Die [Abnahme zu 5534ff0](https://github.com/Lulus792/SecondBrain/actions/runs/37466622105) besteht mit allen 18 Jobs.
Alle sechs Desktop-Jobs führen sieben Prüfungen in Debug/Release aus. Die drei
Release-Jobs bestehen zusätzlich beide Bedienwege aus den tatsächlich entpackten
Paketen auf Windows x64, macOS ARM64 und Linux x64. Lokal besteht außerdem das
entpackte Intel-macOS-Paket aus einem neuen Ordner mit Leerzeichen und Umlauten.
`otool -L` zeigt ausschließlich macOS-Systembibliotheken. Die älteren verlinkten
Läufe belegen weiterhin ihre jeweils benannten Stände.

## Version 0.2.1: flüssige Navigation und UI-Verfeinerung

Die App öffnet mit Pfeilen in der Sternkarte direkt die nächste Notiz. Ein offener
Entwurf bleibt durch den Schutzdialog gesichert. Kamera, Zoom und Scrollbereiche
verwenden kurze Übergänge anhand verstrichener Zeit; neue Eingaben ändern das Ziel
während der Bewegung. „Bewegung reduzieren“ schaltet sie in der App aus. Suchhinweis
und Bedienelemente der Kopf- und Kameraleiste sind vertikal ausgerichtet.

Das frühere fremde Projektbeispiel und alle Namensverweise wurden aus der aktuellen
Arbeitsfassung entfernt. Tests und HTML-Studien verwenden neutrale Beispieldaten.
Die README wurde nach Recherche von neun GitHub-Projekten neu aufgebaut und zeigt
eine echte App-Aufnahme. Die [Release-Liste](RELEASE.md) beschreibt offene Aufgaben
vor 1.0; die abschließende sprachliche Prüfung ist dafür vorgesehen.

Auf Intel macOS 14.6.1 bestehen alle sieben Release-Prüfungen und neun Python-Prüfungen.
Der Maus-Bedienweg prüft 93 Aussagen, der reine Tastaturweg 99. Neue Prüfungen erfassen
sofortiges Öffnen mit und ohne Entwurf, Zwischenstände der Kamerabewegung, weiches
Scrollen, halbe Mausradschritte, Richtungswechsel und reduzierte Bewegung. Große und
kleine tatsächliche Darstellungen wurden betrachtet. Dieselben sieben Tests bestehen
mit AddressSanitizer und UndefinedBehaviorSanitizer. Das tatsächlich entpackte und
verschobene Intel-macOS-Paket besteht beide Bedienwege mit 93 und 99 Aussagen.

Die [Abnahme zu 29b15d1](https://github.com/Lulus792/SecondBrain/actions/runs/37479319260) ist mit allen 18 Jobs erfolgreich abgeschlossen. Desktop Debug/Release mit jeweils
sieben Prüfungen und beide Bedienwege aus entpackten Release-Paketen bestehen auf
Windows x64, macOS ARM64 und Linux x64. Die CI prüft versteckte native Fenster
mit SDL-Softwarerenderer; manuelle Freigaben aller Geräte und GPU-Treiber sind
damit weiterhin nicht behauptet. Die lokale App unter dist/SecondBrain ist aktualisiert.

## Entwicklungsschritt 0.3.0: Navigation und Karten

Archivdokumente schalten den Bereich nicht mehr ungefragt um. Eine globale
Bereichswahl bleibt erreichbar. Raumfahrt verwendet Weltpositionen und Parallaxe;
Kartenwechsel passen den Bildausschnitt weich an. Die große Leseansicht, integrierte
Suchfeld-Löschung, eigene Vektor-Icons und feste Schließen-Knöpfe sind implementiert.
Menüs unterstützen Auf/Ab und Umschalt+F10 öffnet Dokumentaktionen. Scrollbereiche
verwenden eigene Begrenzung, elastische Rückmeldung und ziehbare Indikatoren.
Projektkontext und Hintergrundnotiz halten getrennte Scrollpositionen.

Lokal bestehen alle sieben Release-Prüfungen: 126 Aussagen im Maus-Bedienweg und
105 im reinen Tastaturweg. Wiederholtes Scrollen am Ende, Archiv-Rückkehr,
Menüpfeile und große Leseansicht sind enthalten. Leisten wurden bei kleinem Fenster
mit 200 Prozent Schrift anhand ihrer tatsächlichen Geometrie geprüft und betrachtet.
Neun Python-Prüfungen bestehen. Dieselben sieben Tests bestehen mit AddressSanitizer und
UndefinedBehaviorSanitizer. Das entpackte und verschobene Intel-macOS-Paket besteht
beide Bedienwege. Die [Abnahme zu c224223](https://github.com/Lulus792/SecondBrain/actions/runs/37494305314) ist mit allen 18 Jobs erfolgreich abgeschlossen. Desktop Debug/Release mit
je sieben Tests und beide Bedienwege aus entpackten Release-Paketen bestehen auf
Windows x64, macOS ARM64 und Linux x64. Die Software- und Geräteabnahme bleibt auf
den dokumentierten Umfang begrenzt.

Dieser Entwicklungsschritt schließt das aktive Gesamtziel nicht ab. Die weiteren
Arbeiten der [Release-Liste](RELEASE.md) bleiben beauftragt; die Versionsnummer
1.0 bleibt bis zur ausdrücklichen Nutzerfreigabe gesperrt. Die eigene MIT-Lizenz
ist festgelegt. Herausgeberzertifikate sind noch nicht vorhanden.


## Entwicklungsschritt 0.3.1: Einstellungen und Ordnerwahl

Die App speichert beim regulären Beenden Arbeitsordner, Projekt, Notiz,
Fenstergröße, Farbdarstellung, Schriftgröße, Transparenz- und Bewegungsschalter.
Ein ausdrücklich übergebener Arbeitsordner hat beim Start Vorrang. Fehlende alte
Ordner werden gemeldet; ihre gespeicherte Angabe bleibt bis zur bewussten
Wiederherstellung erhalten. Eine unbekannte oder beschädigte Konfiguration wird
nicht überschrieben. Der eigene C-Kern implementiert Format und Speicherung.
[Vertrag und Bedienung](EINSTELLUNGEN.md).

Das Arbeitsordnerformular bietet eine native SDL3-Ordnerauswahl. Auswahl übernimmt
zunächst nur den Feldwert. Abbruch erhält ihn; verspätete Antworten eines
veralteten Formulars werden ignoriert. Native Callback-Lebensdauer und UI-Thread-
Übergabe sind abgesichert. Der tatsächliche OS-Dialog ist noch nicht interaktiv
mit Maus, Tastatur oder Screenreader auf allen Zielsystemen abgenommen.

Die neue Prüfung umfasst Format/Bestandsschutz, neu erzeugte Fenster und zwei
getrennte App-Prozesse mit isolierter Konfiguration. Screenshots des kleinen
780 × 560-Fensters bei 150 und 200 Prozent Schrift wurden betrachtet.
Lokal bestehen alle zehn Release-Prüfungen, ein vollständiger Zehn-Test-Lauf mit
AddressSanitizer/UndefinedBehaviorSanitizer sowie die gezielte Nachprüfung der
drei Einstellungsprüfungen nach der letzten Hinweis-/Fehlerfallkorrektur.
Die neue UI-Prüfung enthält 51 Aussagen; neun Python-Prüfungen bestehen.
Crossplatform- und Paketnachweise für diesen Stand folgen erst nach Ausführung.
Die vollständige Release-Liste bleibt offen.


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


## Sicherungskern in Arbeit

Der eigene C-Kern implementiert Inhaltsarchive mit SHA-256, Lesen/Prüfen,
Wiederherstellen unter freiem Namen, exklusives Veröffentlichen und Abbruch-
Aufräumen. Der CLI bietet backup, inspect und restore. Gezielte lokale Prüfungen
umfassen komplette Rundreise, binäre Anhänge, leere Ordner, Unicode, bekannte
Hashvektoren, beschädigte und gefährliche Pfade, Bestandschutz und Schreibfehler
nach Teilfortschritt. Die Desktop-Anbindung ist noch nicht implementiert.
[Vertrag, Prüfumfang und Grenzen](SICHERUNG.md).

Lokal bestehen die sieben betroffenen Prüfungen in Release und mit ASan/UBSan: 482
Integritäts- und 27 Schreibfehleraussagen sowie getrennte Produktions-CLI-Prozesse.
Neue Plattformnachweise folgen erst nach ausgeführten CI-Jobs.


## Entwicklungsschritt 0.4.0: Sicherung in der App

Aktionen bietet Projekt sichern; Projekte und Aktionen bieten Wiederherstellen.
Native Dateiauswahl und Pfadfeld, eigener Hintergrundjob, Abbruch, geprüfte
Vorschau und freier Zielordnername sind implementiert. Die Vorschau ist an die
Gesamtprüfsumme gebunden; eine geänderte Datei wird vor Wiederherstellen abgewiesen.
Ein anderer offener Entwurf bleibt erhalten. Gleichnamige Projekte lassen sich
über die Ordnerkennung unterscheiden. Fehlermeldungen sind lokal sichtbar und
vollständig mit Rückmeldung kopierbar. Status/Abbruch besitzen feste Positionen.

Ein vollständiger lokaler 14-Test-Release-Lauf besteht; anschließend bestehen
gezielte Nachprüfungen des neuen Bedienwegs und Kerns. Der neue Durchlauf umfasst
75 Aussagen und eine 8-MiB-Datei. Tatsächliche normale/beschäftigte Softwarebilder
und kleines Fenster mit 200 Prozent Schrift wurden betrachtet. Neun Python-
Prüfungen bestehen. Ein versehentlicher paralleler UI-Testversuch störte die
Zwischenablage; seine Fehler werden seriell nachgeprüft und als Prüfbetrieb geführt.
Weitere Paket-/Plattformnachweise werden nach Abschluss ergänzt.
[Vertrag und offene Abnahmen](SICHERUNG.md).


Die seriell wiederholten Tastatur-/Sicherungsprüfungen bestehen mit ASan/UBSan
nach dem versehentlichen Clipboard-Überlappungsversuch. Andere zwölf Prüfungen
des vollständigen Sanitizer-Laufs bestanden bereits. Die gezielte Wiederholung
prüft die betroffenen Pfade und letzte Layout-/Workeränderungen; ein neuer
vollständiger ungestörter Sanitizer-Gesamtlauf wird damit nicht behauptet.


## Abnahme 0.4.0

Der [Lauf zu d2d9c2b](https://github.com/Lulus792/SecondBrain/actions/runs/37508856383) besteht mit allen 18 Jobs.
Je 14 Desktopprüfungen bestehen in Debug/Release auf Windows x64, macOS ARM64
und Linux x64. Alle drei tatsächlich entpackten Pakete bestehen 126 Maus-,
105 Tastatur- und 75 Sicherungs-Bedienaussagen, Einstellungsneustart in getrennten
Prozessen und den Produktions-CLI-Sicherungsablauf. Die neue UI-Prüfung verwendet
eine 8-MiB-Datei; Kernintegrität umfasst 487 Aussagen und injizierte Schreibfehler 27.

Lokal besteht das entpackte und verschobene Intel-macOS-Paket ebenfalls. Die App
unter dist/SecondBrain/secondbrain.app ist aktualisiert. Alle 14 Release-Prüfungen
und gezielte Nachprüfungen bestehen. Im Sanitizer-Gesamtlauf bestanden zwölf
Prüfungen; die zwei durch einen versehentlichen Clipboard-Überlappungsversuch
gestörten Wege bestehen in der anschließenden seriellen Wiederholung mit
ASan/UBSan, einschließlich der letzten UI-/Workeränderungen. Neun Python-Tests bestehen.

Die Tests verwenden SDL-Ereignisse und versteckte native Fenster mit
Software-Renderer. Native OS-Dateidialoge, physisch volle Zielvolumes, harter
Prozessabbruch, Screenreader und Geräte-/Langzeitabnahme bleiben offen.
Die vollständige Release-Liste bleibt beauftragt; 1.0 bleibt bis zur
Nutzerfreigabe gesperrt.


## Entwicklungsschritt 0.5.0: native Zugänglichkeit

AccessKit 0.23.1 ist ausschließlich in der UI integriert. Alle drei nativen
Adapter werden mit der SDL-Fensterlebensdauer verbunden. Eigene C-Snapshots
beschreiben Bedienelemente, Editor, Lesewert und Dokumentsterne. Aktionen werden
begrenzt gepuffert, auf dem UI-Thread geprüft und nach Kontextwechsel verworfen.
Windows erhält die zugehörige UI-DLL im Paket; Original-Lizenzen sind enthalten.

Die lokale Intel-macOS-Providerprüfung besteht für native Schaltflächen,
Texteingaben, Notizerstellung, Bearbeiten/Speichern, Textauswahl und Lesewerte.
Ein gezielter Test verhindert die Anwendung verspäteter Änderungen auf ein
anderes Dokument. Der plattformübergreifende Vertrag prüft außerdem ungültiges
UTF-8, Auswahlgrenzen, Schreibschutz, neue Knotenkennungen und Queue-Grenzen.
Die Gesamt-/Plattformabnahme dieses Schritts folgt nach den laufenden Prüfungen.

Offen bleiben tatsächliche UIA-/AT-SPI-Clientabfragen, VoiceOver/NVDA/Orca-Bedienung,
vollständige passive Dialogtexte, Grapheme/Zeilengeometrie und Leistung großer
Dokumente. Die native macOS-Methode für Auswahlersetzung ist im verwendeten
Adapter nicht verfügbar; Setzen des Gesamtwerts und Auswahl sind geprüft.
[Details und Grenzen](BARRIEREFREIHEIT_PLAN.md). 1.0 bleibt gesperrt.


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


## 0.5.1: zugängliche Dialog- und Hilfetexte

Statische Hinweise, Einführung, leere Listen, Sicherungsvorschau/-fehler,
Tastaturhilfe und Entwurfswarnung werden mit ihren sichtbaren UI-Texten in den
nativen Baum übernommen. Dialogtitel benennen ihre Aufgabe; passive Texte
tragen keine bedienbaren Aktionen. Innerhalb der UI-Gruppen entspricht die
Reihenfolge der Zeichnung. Modalinhalte entfernen Hintergrundtexte/-aktionen.
Die Kontextkennung entspricht dem tatsächlich gezeichneten Formularzustand.

Die lokale Providerprüfung besteht für vollständige Hilfezeile, Titel-Identifier,
Entwurfswarnung, Abschirmung und Rückkehr. Der gemeinsame Vertrag prüft 700-Zeichen-
Beschriftungen und abgelehnte Fokus-/Klick-Aktionen auf passivem Text. Die erste
macOS-Rollenprüfung nahm fälschlich AXHeading als Zeichenfolge an; der ältere
lokale Adapter liefert Heading, neuere Systeme werden gegen ihre Systemkonstante
geprüft. Das ist kein Beleg menschlicher VoiceOver-Überschriftennavigation.
Gesamt-/Plattform-/Sanitizerabnahme läuft noch. [Details](BARRIEREFREIHEIT_PLAN.md).


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


## 0.6.0: Systemvorgaben für Darstellung und Bewegung

Eine eigene C-UI-Anbindung liest native Zugänglichkeitsvorgaben. macOS verwendet
NSWorkspace, Windows SystemParametersInfoW, Linux das standardisierte Settings-
Portal im Hintergrund und unter GNOME ein verfügbares Schema für enable-animations.
Die Lumen-Auswahl bleibt Standard; Systemdarstellung ist separat wählbar.
Systemvorgaben ergänzen die eigene Auswahl, ohne ihre gespeicherten Werte zu
ersetzen. Erhöhter Kontrast verwendet Deckflächen, stärkere Ränder, Fokus und
klarere Textfarben. Ein erster zu heller Hinweis in der hellen Kontrastansicht
wurde nach tatsächlicher Bildbetrachtung korrigiert.

Formatversion 2 ergänzt Systemdarstellung und Kontrast. Alte vollständige
Version-1-Dateien werden gelesen und erst beim Speichern aktualisiert. Die
[Abnahme zu 444471d](https://github.com/Lulus792/SecondBrain/actions/runs/37527601503)
besteht mit allen 18 Jobs: 16 Desktoptests je Debug/Release auf Windows x64,
macOS ARM64 und Linux x64 sowie die drei entpackten Pakete. Lokal bestehen alle
16 Release-Tests (191,75 Sekunden) und alle 16 ASan/UBSan-Tests (398,57 Sekunden).
Die native Mac-Abfrage liest die vorhandenen Vorgaben, ohne sie zu ändern; Linux
prüft das tatsächliche D-Bus-Protokoll mit einem privaten Testportal. Diese
Prüfwege ersetzen keine manuelle Abnahme echter Systemsteuerungswechsel. [Vertrag und Grenzen](EINSTELLUNGEN.md).


Das entpackte Intel-Paket 0.6.0 besteht lokal mit 126 Desktop-, 105 Tastatur- und
75 Sicherungs-UI-Aussagen sowie zwei Einstellungsprozessen und dem CLI-Sicherungs-
ablauf in Unicode-Pfaden. dist/SecondBrain ist auf 0.6.0 aktualisiert; der Start
mit dem eigenen Projektgedächtnis und das tatsächliche Bild sind geprüft.
Archiv-SHA-256: `897c24cea9a2bea02575c60afd7697d6ed73dce7da07d9a16fe43d84c50f173e`.
Die Pakete bleiben Entwicklungspakete ohne Herausgeberzertifikat. 1.0 bleibt gesperrt.


## 0.6.1: Kontrast der funktionalen Sternkarte

Nicht ausgewählte Sterne und Verbindungen waren auf hellen Kontrastflächen
zu blass. Der Kontrastmodus zeichnet Sternkerne jetzt deckend und Verbindungen
mit klarer Graustufe und breiterem Strich. Beschriftungen verwenden die
Kontrasttextfarbe und verdecken Linien unter ihrem Textfeld. Die normale
Lumen-Darstellung behält ihre Akzentfarben und ihr Glühen. Systemdarstellung
und Kontrastwahl erhalten außerdem ein Icon aus dem eigenen Satz.

Die Regression misst tatsächliche Rasterpixel für ungewählte Sterne und Kanten
auf Schwarz und Weiß, einschließlich der verkleinerten Rasterdarstellung bei
2560 logischen Pixeln. Die Mindestanforderung 3:1 ist eine eigene Übertragung
von [W3C Nicht-Text-Kontrast](https://www.w3.org/WAI/WCAG22/Understanding/non-text-contrast.html),
gelesen am 6. Oktober 2026. Dünne Rasterlinien werden zusätzlich verbreitert.
Die Prüfung ist keine vollständige Kontrastfreigabe aller App-Zustände.


Der lokale Gesamtprüflauf besteht mit allen 16 Tests (158,93 Sekunden). Eine
anschließende Ergänzung prüft Tastatur-Reichweite der Kontrastoption bei 200 Prozent
Schriftgröße. Die Bildkontrolle zeigte dabei einen unzureichend unterscheidbaren
Fokusring; der Kontrastmodus erhält einen getrennten inneren Ring. Die abschließende
Nachprüfung und Plattformresultate werden gesondert ergänzt.


Abschließende lokale Nachprüfung: system-appearance mit 51 Aussagen und der
vollständige Tastaturablauf bestehen (62,22 Sekunden). ASan/UBSan bestehen für
system-appearance und ui-rendering-editor (26,03 Sekunden). Die tatsächlichen
hellen/dunklen Bilder und der fokussierte Einstellungsdialog bei 200 Prozent
Schrift wurden betrachtet. Die native CI-Abnahme zu diesem Schritt steht noch aus.


## Abnahme der Kontrastnacharbeit 0.6.1

[Lauf zu b064bd5](https://github.com/Lulus792/SecondBrain/actions/runs/37529776082)
besteht mit allen 18 Jobs: 16 Desktoptests je Debug/Release auf Windows x64,
macOS ARM64 und Linux x64 sowie drei entpackte Pakete. Lokal besteht das entpackte
Intel-Paket mit 126 Desktop-, 105 Tastatur- und 75 Sicherungs-UI-Aussagen sowie
Einstellungsprozessen und CLI-Sicherung. Sein tatsächlicher Start wurde betrachtet.
dist/SecondBrain ist auf 0.6.1 aktualisiert. Archiv-SHA-256:
`77dc5b20ccbd95d9a50bb652a59d937952eeba10e418cf08b0846dcc4055cee8`.
Vollständige assistive Bedienung und Kontrastabnahme aller Zustände bleiben offen.


## 0.7.0: Verständlicher leerer Einstieg

Ohne geöffnetes Projekt zeigt eine zentrale Karte direkte Wege zum Anlegen,
Öffnen, Wiederherstellen, zur Hilfe und Darstellung. Speicherort und kurze
Erklärung ersetzen die Dokumentwerkzeuge des leeren Zustands. Große Schrift
verwendet einen festen Titel und scrollbare Aktionen. Dialogabbruch erhält den
Ursprungsfokus; das betrifft auch während des Frame-Aufbaus geöffnete Fenster.
[Recherche, Verhalten und Grenzen](ERSTER_START.md).

Alle 17 lokalen Release-Tests bestehen (177,06 Sekunden). Die tatsächlichen Bilder
zeigen Lumen und die helle Kontrastansicht bei 200 Prozent Schrift. Abschließende
Mausrad-/Sanitizer- und native Plattformabnahmen folgen gesondert. Der erste
Größentest wurde auf die tatsächlich vorhandene Mindestgröße 780 × 520 korrigiert.


Abschließende lokale Nachprüfung: erster Start mit 40 Aussagen einschließlich
Mausrad innerhalb/außerhalb der Karte besteht (5,56 Sekunden). ASan/UBSan bestehen
für ersten Start, Systemdarstellung und native Zugänglichkeit (42,09 Sekunden;
40/51/121 Aussagen). Der volle lokale Release-Lauf enthält 17 bestandene Tests.
Die Plattform- und entpackte Paketabnahme zu 0.7.0 steht noch aus.


## Abnahme des Einstiegs 0.7.0

[Lauf zu beba1e5](https://github.com/Lulus792/SecondBrain/actions/runs/37531214816)
besteht mit allen 18 Jobs: 17 Desktoptests je Debug/Release auf Windows x64,
macOS ARM64 und Linux x64 sowie drei entpackte Pakete. Lokal besteht das entpackte
Intel-Paket mit 126 Desktop-, 105 Tastatur- und 75 Sicherungs-UI-Aussagen sowie
Einstellungsprozessen und CLI-Sicherung. Archiv-SHA-256: `335f869ca5f4e3403ea21fa54abc148245d84b2e241e1dd846d4a2d71bb08b1e`.
Menschliche assistive Bedienung und echte native Dialogbedienung bleiben offen.


## 0.7.1: Metadaten- und Textvertrag

Die Metadatenprüfung erhält bisher lesbare Minimaldateien und prüft bekannte
Felder, doppelte Schlüssel und Schema 1. Unbekannte Schemas werden gemeldet;
Vorlagenversionen lösen kein Überschreiben aus. Projektliste und Sicherungen
verwenden dieselbe Prüfung. Längenbasierte APIs kopieren exakt die angegebenen
Bytes, ohne außerhalb eines Eingabepuffers nach einem Terminator zu suchen.

Notizen, Dokumentliste und externe Quellen lehnen NUL-Zeichen vor der Verwendung
als C-Text ab. Damit kann ein unsichtbarer Dateirest nicht durch Editieren verloren
gehen. Originaldatei, bisherige Quelle und offener Entwurf bleiben erhalten.
[Vertrag und offene Grenzen](DATENVERTRAG.md).

Alle acht UI-unabhängigen Kerntests und vier gezielte ASan/UBSan-Prüfungen bestehen.
Der neue Modelltest korrigiert seine relative Quelle: bei bereits geöffneter Quelle
ist deren Ordner die Basis. Der vollständige Desktop- und Plattformnachweis folgt.


Abschließender lokaler Nachweis: Der vollständige Desktoplauf prüfte alle 18
Tests; die neue Quellenregression hatte zunächst einen falschen relativen Link
und wurde korrigiert. Die abschließenden vier Release-Nachprüfungen bestehen
(0,49 Sekunden), einschließlich 122 Metadaten- und 89 Modellaussagen. Alle acht
reinen Kerntests bestehen (3,60 Sekunden); die vier gezielten ASan/UBSan-Wege
bestehen (2,68 Sekunden). Mit dem bisherigen Kern schlägt die neue Regression
beim unbekannten Schema 2 fehl (Exit 1). Plattform- und Paketabnahme zu 0.7.1
folgen gesondert. Es fand keine automatische Migration bestehender Daten statt.

Das vollständig geprüfte Paket 0.7.0 wurde lokal nach dist/SecondBrain übernommen;
sein tatsächlicher Start mit dem eigenen Projektgedächtnis ist betrachtet.


## Abnahme von 0.7.1

[Lauf zu b3c0af5](https://github.com/Lulus792/SecondBrain/actions/runs/37532681504)
besteht mit allen 18 Jobs: 18 Desktoptests je Debug/Release auf Windows x64,
macOS ARM64 und Linux x64 sowie drei entpackte Pakete. Lokal besteht das entpackte
Intel-Paket mit 126 Desktop-, 105 Tastatur- und 75 Sicherungs-UI-Aussagen sowie
Einstellungsprozessen und CLI-Sicherung. Archiv-SHA-256: `6bd2cea886e7e6f88335b8987cd1e25839f3b0b54ff969e84afe403114d24aba`.
Der weitere Release-Auftrag einschließlich Einzelprojekt-Fehlerzuständen bleibt aktiv.


## 0.7.2: Einzelne Projektfehler erhalten die weitere Arbeit

Die App erkennt gültige Projekte und nicht verfügbare Einträge getrennt. Die
Projektwahl nennt Grund und Pfad, die Werkzeugleiste zeigt die Anzahl. Erneutes
Prüfen erhält den offenen Entwurf. Korrigierte Einträge werden wieder wählbar;
sind alle nicht verfügbar, bleiben die Wege des Einstiegs erreichbar. Ein Fehler
beim Öffnen von Notizen blockiert nicht den nächsten gültigen Kandidaten. Aktuelle
Metadaten werden vor dem Projektwechsel erneut gelesen. Die strikte Kernfunktion
bleibt für bestehende Aufrufer erhalten. [Vertrag](DATENVERTRAG.md).

Die Bild-/Tastaturprüfung bei 200 Prozent Schrift fand fehlendes Fokus-Reveal nach
Größenänderung. Die App führt es jetzt auch nach Fenster-/Schriftänderungen aus.
Fehlertexte und Pfade umbrechen; die erneute Prüfung erhält eine normale Knopfhöhe.
Gezielte Modell-/UI- und neun reine Kerntests bestehen. Gesamtlauf, abschließende
Sanitizer- und Plattformabnahme folgen gesondert.


Abschließender lokaler Nachweis: alle 20 Tests des vollständigen Release-Laufs
bestehen (190,75 Sekunden), der zusätzliche Produktions-CLI-Test besteht
(0,77 Sekunden). Alle zehn reinen Kerntests bestehen (3,72 Sekunden). Vier gezielte
ASan/UBSan-Wege bestehen (38,51 Sekunden): Projektfehler-UI, erster Start, native
Zugänglichkeit und Projektmodell. Die neuen Prüfungen enthalten 39 UI- und
46 Modell-/Erkennungsaussagen. Tatsächliche dunkle und helle Kontrastbilder mit
200 Prozent Schrift sind betrachtet. Native Plattform- und Paketabnahme zu 0.7.2
folgen gesondert; automatische Metadatenreparatur wurde nicht eingeführt.

Das lokal abgenommene Paket 0.7.1 liegt unter dist/SecondBrain. Der tatsächliche
Start mit dem eigenen Projektgedächtnis ist betrachtet; Version 0.7.2 folgt erst
nach ihrer gesonderten Paketabnahme.


## 0.7.3: Aktuelle Metadaten vor Schreibaktionen prüfen

Auch ein bereits geöffnetes Projekt kann extern nicht unterstützte Metadaten
bekommen. Die Schreibwege prüfen nun den aktuellen Stand: Speichern, neue Notizen,
Kopien und Archivieren. Kurz vor Ersetzen/Verschieben wird die Metadatenrevision
zusätzlich abgeglichen. Erkanntes Schemaproblem/Entfernen schützt Datei und Entwurf;
fehlgeschlagenes Save im Wechsel-Dialog erhält die Entscheidung. Gültige spätere
Metadaten ermöglichen die Aktion wieder. [Vertrag und Grenzen](DATENVERTRAG.md).

Alle elf reinen Kerntests bestehen. Gezielte Sanitizer-, Desktop- und native
Plattformnachweise folgen gesondert. Die Regression verwendet private Testdaten.


## Abnahme von 0.7.2

[Lauf zu f2e2725](https://github.com/Lulus792/SecondBrain/actions/runs/37534667024)
besteht mit allen 18 Jobs: 21 Desktoptests je Debug/Release auf Windows x64,
macOS ARM64 und Linux x64 sowie drei entpackte Pakete. Die Teilfehlerbehandlung
besteht auch im Produktions-CLI-Prozesstest auf den nativen CI-Systemen. Lokal
besteht das entpackte Intel-Paket mit 126 Desktop-, 105 Tastatur- und 75 Sicherungs-
UI-Aussagen sowie Einstellungsprozessen und CLI-Sicherung. Archiv-SHA-256:
`9989fc424de596e021504e0ac71132231d20347d4e923cc68ab82e81f38a8c1d`. Menschliche assistive Bedienung und übrige Release-Abnahmen bleiben offen.


Abschließender lokaler Nachweis: 22 Release-Tests bestehen (302,43 Sekunden),
zusätzlich die ergänzte UI-Nachprüfung (14,47 Sekunden). Elf reine Kerntests
bestehen (2,41 Sekunden). Fünf gezielte ASan/UBSan-Wege bestehen (8,45 Sekunden),
die ergänzte UI-/Guard-Nachprüfung ebenfalls (22,27 Sekunden). Die Regression
enthält 75 Guard-Aussagen; die UI prüft Save-Verweigerung und Wiederkehr gültiger
Metadaten innerhalb ihrer 47 Aussagen. Das tatsächliche Fehlerbild wurde betrachtet.
Mit dem bisherigen Kern schlägt die Guard-Regression beim Speichern fehl (Exit 1).
Native Plattform- und entpackte Paketabnahme zu 0.7.3 folgen gesondert.

Das abgenommene Intel-Paket 0.7.2 ist nach dist/SecondBrain übernommen. Sein
tatsächlicher Start mit dem eigenen Projektgedächtnis ist betrachtet.


## Abnahme von 0.7.3

[Lauf zu 7331eca](https://github.com/Lulus792/SecondBrain/actions/runs/37536230101)
besteht mit allen 18 Jobs: 22 Desktoptests je Debug/Release auf Windows x64,
macOS ARM64 und Linux x64 sowie drei entpackte Pakete. Lokal besteht das entpackte
Intel-Paket mit 126 Desktop-, 105 Tastatur- und 75 Sicherungs-UI-Aussagen sowie
Einstellungsprozessen und CLI-Sicherung. Archiv-SHA-256:
`ce836ac9efc670a6a515539197387179b1a47c0df12183306de3f8f28e88f404`. Die vollständige Release-Arbeit bleibt offen.


## Dauerhafte Vorabversionen vorbereiten

Der neue Tag-Workflow verwendet den vollständigen Plattformtest als wiederverwendbaren
Workflow und erhält einen Intel-Mac-Lauf zusätzlich zu den bisherigen Architekturen.
Versionsprüfung verweigert automatisch 1.0 und abweichende Tags. Vier Archive und
SHA256SUMS werden nur nach Paketabnahme hochgeladen und anschließend wieder gegen
die geprüften Bytes verglichen. Erst danach wird ein Release-Entwurf veröffentlicht.
Vorhandene Releases werden nicht ersetzt. Tag-/Manifestregression und actionlint
1.7.12 bestehen lokal. Tatsächliche Veröffentlichung und Intel-CI-Abnahme folgen.

Alle zwölf reinen Kerntests einschließlich des Veröffentlichungsschutzes bestehen
(4,86 Sekunden). actionlint 1.7.12 meldet keine Workflowfehler. Dies bestätigt die
lokale Vorbereitung, noch keine tatsächliche Release-Veröffentlichung.

## Versionsangaben, 7. Oktober 2026

0.8.0 ergänzt „Über SecondBrain“ unter Darstellung. Die Karte zeigt Version,
Entwicklungsstatus und Buildangaben; Kopieren übernimmt ausschließlich diese
Angaben. App und CLI unterstützen `--version` vor Initialisierung von UI,
Einstellungen und Projektdateien. CMake erzeugt die Version aus PROJECT_VERSION
und erfasst Quellrevision, Konfiguration, System und Architektur. Ein Build mit
geänderten versionierten Dateien trägt `-dirty`; ohne eigenes Git-Repository
lautet die Quellkennung `source`. Die Kennung stammt vom Konfigurieren,
nicht von einer späteren dynamischen Prüfung.

13 lokale Debug-Kerntests bestehen. Die ersten drei Release-Nachprüfungen
(Versionen und Tastaturablauf) bestehen; die Bildprüfung deckte ungültig
gezeichnete Zeilenumbrüche auf, die anschließend in einzelne Zeilen aufgeteilt
wurden. Abschließendes Bild ist geprüft. Vier gezielte Release-Nachprüfungen bestehen
(Versionen, erster Start und bestehende native macOS-Providerprüfung; 10,15 Sekunden).
Das korrigierte entpackte Intel-Paket besteht mit 126 Desktop-, 112 Tastatur-
und 75 Sicherungs-UI-Aussagen, Einstellungsneustart und CLI-Sicherung.
SHA-256: `604ab4a379448f6c984f255d6089328e05924303662738c34f5abd43ac4e9459`.
Ein Quellbaum ohne eigenes Git-Repository baut und meldet korrekt `Build: source`.
Native 0.8.0-Plattformabnahme folgt.
Die Paketprüfung vergleicht zusätzlich Archivversion, App, CLI und die beiden
macOS-Bundle-Versionsfelder. Menschen mit Screenreadern und die native
Menüleiste sind damit noch nicht abgenommen.

## Dauerhafte Vorabversion v0.7.3

[Lauf37537383565](https://github.com/Lulus792/SecondBrain/actions/runs/37537383565)
besteht mit 22 Jobs: Versionsprüfung, 20 wiederverwendete Plattformjobs und
Veröffentlichung. Desktop-Prüfungen laufen je Debug/Release auf Windows x64,
Linux x64, macOS ARM64 und Intel; die vier Release-Pakete wurden entpackt und
geprüft. [v0.7.3](https://github.com/Lulus792/SecondBrain/releases/tag/v0.7.3)
ist öffentlich, als Vorabversion gekennzeichnet und enthält vier Archive sowie
SHA256SUMS. Die Pipeline prüft heruntergeladene Uploads vor Veröffentlichung
gegen die ursprünglichen Paketprüfsummen. Signierung und Notarisierung fehlen
weiterhin; dies ist keine 1.0-Freigabe.

Die vier öffentlich heruntergeladenen 0.7.3-Archive stimmen lokal mit SHA256SUMS
und den von GitHub gemeldeten SHA-256-Digests überein; auch der Manifest-Digest
ist geprüft. Das ersetzt keine erneute lokale Geräteabnahme aller Systeme.

`dist/SecondBrain` ist lokal auf das geprüfte Intel-Paket 0.8.0 aktualisiert.
Versionsausgabe und tatsächlicher Start mit dem eigenen Projektgedächtnis
bestehen. Der lokale Entwicklungsbuild meldet seine damalige Quellrevision
mit `-dirty`; die späteren CI-Pakete erfassen ihren eigenen sauberen Commit.

## Windows-Zwischenablage, 7. Oktober 2026

Der erste 0.8.0-Lauf37539685660 zu ae9ee97 findet unter Windows im neuen
Kopiertest den CF_UNICODETEXT-Zeilenwechsel: SDL3 ergänzt CR vor LF. Beide
Versionsprozesstests bestehen; der Bytevergleich der Zwischenablage schlägt fehl.
Original: src/video/windows/SDL_windowsclipboard.c der geladenen SDL3 3.2.30.
Der Vergleich akzeptiert nun LF oder CRLF, erhält aber die vollständige Prüfung
aller übrigen Zeichen. Sechs C-Prüffälle bestehen lokal: beide korrekten Formen,
ein falscher Buchstabe, zusätzliches Zeichen, alleinstehendes CR und NULL.
Native Wiederholungsabnahme folgt; kein erfolgreicher 0.8.0-Gesamtlauf behauptet.

## Native Abnahme von 0.8.0

[Lauf37540475172](https://github.com/Lulus792/SecondBrain/actions/runs/37540475172)
zu 1ab4ab5 besteht mit 20 Jobs, 25 Desktoptests je Debug/Release auf Windows x64,
Linux x64 sowie macOS ARM64/Intel und vier entpackten Release-Paketen. Der
Windows-CRLF-Vergleich ist damit tatsächlich abgenommen. Das ist kein Nachweis
für die nachfolgende Textanbindung 0.9.0.

## Geformter Text und Ersatzschriften in 0.9.0

[Textdarstellung](TEXTDARSTELLUNG.md) dokumentiert Recherche, UI-Anbindung und
verbleibende Arbeit. SDL_ttf, FreeType und HarfBuzz verwenden festgelegte
Archive/Submodulstände; der eigene Code ist C17 und der reine Kern bleibt
unabhängig von der UI. Noto-Ersatzschriften ergänzen Arabisch, Hebräisch,
Devanagari, Symbols2 und CJK/Kana/Hangul. Die Lizenztexte liegen auch innerhalb
der Anwendungsressourcen.

Alle 26 lokalen Release-Tests bestehen (262,08 Sekunden). Nach der aus dem Bild
abgeleiteten Größenkorrektur der Ersatzschriften bestehen drei gezielte
Nachprüfungen (6,13 Sekunden). Das endgültige entpackte Intel-Paket besteht mit
126 Desktop-, 112 Tastatur- und 75 Sicherungs-UI-Aussagen, Einstellungen und
CLI-Sicherung. 13 neu konfigurierte Debug-Kerntests bestehen (5,93 Sekunden);
deren Build konfiguriert ausschließlich C und enthält keine Textbibliothek.
Der neue Texttest scheitert mit dem bisherigen Renderer an fehlender
arabischer Verbindung (Exit 1). Das korrigierte echte Rasterbild zeigt die
geprüften Schriften bei 200 Prozent. Native 0.9.0-Plattformabnahme folgt.

Gemischte Schreibrichtungen und Scriptwechsel, weitere Schriften/Emoji,
graphemgenaue Bearbeitung und native Textgeometrie bleiben offen; dieser
Schritt bestätigt weder eine vollständige Sprach- noch Screenreader-Abnahme.

Gezielte ASan-/UBSan-Ausführung des eigenen Textmoduls, UI-Klebers und
Texttests besteht mit 19 Aussagen (Verbindung, CJK/Hangul, Akzent und drei
Schriftgrößen). Die externen statischen Bibliotheken sind dabei nicht
instrumentiert; LeakSanitizer ist auf diesem Mac nicht Teil der Prüfung.
Die kleine normale Textszene läuft in 0,37 Sekunden mit maximal 89.182.208
Bytes RSS (~85 MiB). Das ist kein Langzeit- oder Großdaten-Leistungsnachweis.

Der geprüfte 0.9.0-Build ist nach dist/SecondBrain übernommen. Version und
Standbild mit dem eigenen Projektgedächtnis bestehen. Die lokale Buildkennung
nennt die beim Konfigurieren erfasste Revision 1ab4ab5 mit `-dirty`; der
[neue Plattformlauf zu 3161f08](https://github.com/Lulus792/SecondBrain/actions/runs/37543159232)
prüft den sauberen Commit gesondert.

## Strukturierte Leseansicht 0.9.1, 7. Oktober 2026

[Dokumentstruktur](DOKUMENTSTRUKTUR.md) beschreibt den erkannten Markdown-
Umfang, Blöcke, natürliche Überschriftenebenen, deren nullbasierte AccessKit-
Abbildung und Alt+Bild-auf/ab. Native Abschnittsanfragen scrollen über dieselbe
Quellposition; Aufgabenwechsel und verborgene/bearbeitete Ansichten verwerfen
alte Anfragen. Vollständige Absätze bleiben auch außerhalb des Viewports
im Dokumentbaum. Ein zusätzlicher kompletter Markdown-Textlauf wird bei
strukturierter Ausgabe vermieden.

Alle 26 lokalen Release-Tests bestehen (270,90 Sekunden). Die abschließende
native macOS-/Vertragsprüfung besteht (9,85 Sekunden): Rollen, tatsächlicher
Textbereich über 6.000 Zeichen ohne doppelte Quelle, echter AXScrollToVisible-
Aufruf, Tastaturziele/Sichtbarkeit, Modalabschirmung, 2.000 Absätze mit stabilen
Kennungen und unveränderten schreibgeschützten Inhalten. Das abschließende entpackte Intel-Paket besteht mit 126 Desktop-, 112 Tastatur-
und 75 Sicherungs-UI-Aussagen sowie Einstellungen und CLI-Sicherung.
Neue native Windows/Linux/macOS-ARM-Abnahme folgt. Zeichen-/Graphemgeometrie,
Listen-/Tabellensemantik und menschliche assistive Abnahme bleiben offen.

Die frühere 0.9.0-Textanbindung zu 3161f08 besteht inzwischen mit 20 Jobs
und vier Paketen; [Originalnachweis](PLATTFORMEN.md).

Gezieltes ASan/UBSan für den eigenen Baumaufbau, die Kennungsverwaltung,
Desktop-Anbindung und die Providerprüfung besteht mit 149 Aussagen. Externe
Bibliotheken sind uninstrumentiert; LeakSanitizer ist nicht Teil dieser Prüfung.
Das geprüfte Paket ist lokal nach dist/SecondBrain übernommen; Version und
Standbild mit dem eigenen Projektgedächtnis bestehen.

## Prozessabbruch, Speicherfehler und AT-SPI-Ebenen in 0.9.2

Alle 15 lokalen Debug-Kerntests bestehen (10,54 Sekunden). Sie enthalten
sechs echte Prozess-Kills in Sicherung/Wiederherstellung mit bytegleichen
Originalen, erhaltenen temporären Resten, sicheren Wiederholungen und
geprüfter manueller Wiederherstellung eines vollständigen temporären Archivs.
Auf einem separat erstellten 32-MiB-HFS+-Volume bestehen zusätzlich echte
ENOSPC-Fehler nach Teilfortschritt in beiden Abläufen und erfolgreiche
Wiederholungen nach Platzfreigabe; das Volume wurde getrennt. Keine
Stromausfall-/Hardwarepersistenz daraus abgeleitet.

Fünf lokale Release-Nachprüfungen bestehen (15,16 Sekunden): native
Zugänglichkeit, Prozess-Kill, echtes Volume und beide Versionsausgaben.
Die neuen Kindprozesse/Orchestrierungen sind reine Testwerkzeuge und werden
nicht in Runtime-Pakete aufgenommen. Normale App-/CLI-Nutzung benötigt
kein Python; beim Bauen mit Tests wird Python 3 benötigt.

Der 0.9.1-Lauf37547431405 zu ad49f03 besteht in 18 Jobs einschließlich
Windows, macOS ARM64/Intel und deren Paketen; beide Linux-Desktopjobs
scheitern am fehlenden nativen AT-SPI-Ebenenattribut. Der Originalquelltext
von accesskit_atspi_common 0.21.0 gibt die gemeinsame level-Eigenschaft
nicht aus. 0.9.2 ergänzt sie mit natürlicher Zählung in einer gehashten
UI-Quellkorrektur. Cargo-Typprüfung besteht (30,83 Sekunden). Die
Vorbereitung ist bytegleich wiederholbar und weist unbekannte Quellen ab.
Neue native AT-SPI-, Prozess- und Volume-Abnahmen folgen im Plattformlauf.

Die native Linux-Nachprüfung zu b69223c besteht inzwischen in Debug und Release,
einschließlich des entpackten Pakets. Der Lauf scheitert unabhängig davon in
sechs Python-Jobs an importseitig gestarteten Prozessskripten. 1358794 behebt
die Testsuche ohne Produktänderung; neun lokale Generatorprüfungen und beide
Prozess-/Volume-Prüfungen bestehen. [Plattformstand](PLATTFORMEN.md).

## Lizenzansicht 0.9.3

Die Versionskarte öffnet eine Übersicht mit 17 mitgelieferten Originaltexten
und einen eigenen unverändernden Lesebereich. Zurück, Kopieren und Schließen
sind mit Tastatur erreichbar; ein Notizentwurf bleibt erhalten. Fehlende oder
ungültige Lizenzdateien zeigen einen Fehler. [Bedienung und Recherche](LIZENZEN.md).

Alle 29 lokalen Release-Tests bestehen (418,13 Sekunden), einschließlich 54
Lizenzressourcen-Aussagen, 240 Tastatur-Aussagen im vollständigen Erstlauf über
alle Texte und 149 nativen/Vertrags-Aussagen. Neun Generatorprüfungen bestehen.
Reale App-Bilder bei 780×560 und 150 Prozent Schriftgröße sind betrachtet.
Eine gefundene gemeinsame Scrollzuordnung von verdeckter Notiz und Lizenztext
ist korrigiert. Die regelmäßige UI-Prüfung benutzt nun ersten, langen Apache-
und letzten Eintrag; der Ressourcentest prüft weiterhin alle 17 Dateien.
Die neue Plattformabnahme und die vollständige Lizenz-/Supportprüfung sind
getrennte offene Schritte; kein 1.0- oder menschlicher Screenreader-Nachweis.

Die wiederholte 0.9.2-Plattformabnahme zu 1358794 ist inzwischen vollständig:
20 erfolgreiche Jobs, acht Desktopprüfungen in Debug/Release und vier entpackte
Pakete. [Konkreter Umfang](PLATTFORMEN.md). Dies ist kein 0.9.3-Nachweis.

Die abschließende Tastatur-Nachprüfung mit drei Einträgen besteht mit 142
Aussagen (115,64 Sekunden). Eine gezielte Ressourcen-Nachprüfung erhält zudem
Backslashes als echte Namenszeichen auf POSIX; auf Windows werden beide
Trennzeichen unterstützt. Das zuerst entpackte Intel-Paket besteht mit 126
Desktop-, 142 Tastatur- und 75 Sicherungs-UI-Aussagen, Einstellungsneustart und
CLI-Sicherung. Nach der kleinen Pfadkorrektur folgt die letzte Paketwiederholung.

Die letzte Intel-Paketwiederholung nach der Pfadkorrektur besteht: 126 Desktop-,
142 Tastatur- und 75 Sicherungs-UI-Aussagen, gespeicherte Einstellungen über
zwei Prozesse und CLI-Sicherung. Archiv-SHA-256:
`5cf73ce1d180bd64fd9e2f1f3fdca35e95ff2c1a91d4e7e6ed4c14653e3a538c`.
Das geprüfte Entwicklungspaket ist lokal nach dist/SecondBrain übernommen;
Versionsausgabe und Standbild mit eigenem Projektgedächtnis bestehen.
Die [neue 0.9.3-Abnahme zu 3f3d3a0](https://github.com/Lulus792/SecondBrain/actions/runs/37550880880)
läuft noch; zwölf Kern-/Python-Jobs bestehen bereits.
