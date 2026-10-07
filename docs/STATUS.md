# Umsetzungsstand von SecondBrain

Stand: 7. Oktober 2026. Die Vorabversion 0.7.3 ist als dauerhafter GitHub Release
veröffentlicht. 0.8.0 mit Versionsangaben ist auf allen vier Paketarchitekturen abgenommen.
Der aktuelle Entwicklungsschritt 0.9.16 ergänzt Referenzlinks und korrigiert den Scroll-/Fokusabstand.
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

Die 0.9.3-Abnahme zu 3f3d3a0 besteht inzwischen mit allen 20 Jobs und vier
entpackten Plattformpaketen. [Umfang](PLATTFORMEN.md). Der folgende Schritt
0.9.4 wird separat geprüft.

## Gemeinsame Dokumentblockregeln 0.9.4

Ein eigener C-Blockleser erkennt Überschriften, Codezäune, eingerückten Code und
Absätze anhand schreibgeschützter Quellbereiche. Titel und Leseansicht verwenden
jetzt dieselben Regeln; CR, CRLF und LF werden erkannt. Code erzeugt keine
scheinbaren Markdown-Überschriften oder Linkaktionen. [Vertrag und Grenzen](MARKDOWN.md).

16 lokale Debug-Kerntests bestehen (34,04 Sekunden). Der erste vollständige
Release-Durchlauf mit 30 Tests besteht (316,43 Sekunden). Die abschließende
Korrektur für eingerückte Absatzfortsetzungen besteht in der separaten Kern-
und nativen UI-Nachprüfung (11,35 Sekunden); ASan/UBSan besteht mit 181.791
Aussagen einschließlich 10.000 deterministischer Eingaben. Die Instrumentierung
betrifft eigenen C-Code; LeakSanitizer ist nicht Bestandteil dieser Prüfung.
Die native macOS-Prüfung bestätigt 177 Aussagen; ein tatsächliches Rasterbild
mit Code, Absatz und Setext-Überschrift ist betrachtet. Die neuen Blockrollen-
Prüfungen scheitern mit der vorherigen Leseansicht aus 368d3b4 bei aktuellem
übrigem Testaufbau. Paket- und neue native Plattformabnahme folgen separat.

Vollständige Container-/Inline-Regeln, Listen-/Tabellensemantik, Textgeometrie,
IME, reale assistive Bedienung und übrige Release-Aufgaben bleiben offen.
Die Version 1.0 bleibt bis zur Nutzerfreigabe gesperrt.

Das endgültige entpackte Intel-Paket besteht mit 126 Desktop-, 142 Tastatur-
und 75 Sicherungs-UI-Aussagen, Einstellungsneustart und CLI-Sicherung. SHA-256:
`436f9ddcc4b56c92cc840cf1d7875c1e774855b97e614a5be63cc59f3d912552`. Das geprüfte Entwicklungspaket wurde nach
dist/SecondBrain übernommen; Version 0.9.4 und eigenes Projekt-Standbild sind
geprüft. Neue native Plattformabnahme folgt nach dem Push.

0.9.4 besteht inzwischen in 19 nativen/Python-Jobs und vier entpackten Paketen.
Windows Debug erreicht die Tastatur-Zeitgrenze; der Gesamtlauf ist nicht grün.
[Genauer Plattformstand](PLATTFORMEN.md).

## Gemeinsame Inline- und Linkregeln 0.9.5

Titel, Leseansicht und Sternkarte verwenden denselben eigenen C-Leser.
Inline-Code und maskierte Zeichen erzeugen keine falschen Aktionen oder
Beziehungen. Ziel und optionaler Linktitel bleiben getrennt; Code-/Bildbeispiele
sowie verschachtelte Links sind geprüft. Links in der ersten Überschrift bleiben
bedienbar. Der KI-Kontext verliert seine erste Projektüberschrift nicht mehr.
[Vertrag und Grenzen](INLINE_LINKS.md).

17 lokale Debug-Kernprüfungen bestehen (39,23 Sekunden); der erste vollständige
Release-Durchlauf mit 31 Tests besteht (318,69 Sekunden). Nach dem zusätzlichen
Grenzfall maskierter erster Backticks und der Überschriftenkorrektur bestehen
vier Kern-/Graph-Nachprüfungen sowie die abschließenden fünf Release-Nachprüfungen
(75,59 Sekunden). Die neue Backtick-Regression scheitert vor der Korrektur.
Gezieltes ASan/UBSan besteht mit 1.117.136 Aussagen einschließlich 5.000
bereichsgeprüfter Eingaben; es instrumentiert eigenen C-Code, nicht die externen
UI-Bibliotheken. LeakSanitizer ist nicht Teil dieser Prüfung.

Die abschließende native macOS-/Vertragsprüfung besteht mit 188 Aussagen:
richtige echte Linkaktionen, Quelle mit optionalem Titel, erhaltene Originale,
erreichbarer Überschriftenlink und sichtbare erste Kontextüberschrift.
Ein tatsächliches Rasterbild mit echten Aktionen und Code-/Maskierungs-Literalen
ist betrachtet. Die Tastatur-Nachprüfung behält 142 Aussagen und alle Aufnahmen;
der letzte Lauf dauert 65,70 Sekunden. Die Diagnostik zeigt bei 600 Testframes
14,36 Sekunden für Eingabe/Layout und 47,99 Sekunden für Rasterung/Präsentation.
Dies sind Prüfwerkzeugdaten, kein normaler App- oder Geräte-Leistungsnachweis.
Neue Plattform- und Paketabnahme folgen separat.

Die neuen Link-/Beziehungsprüfungen scheitern mit der bisherigen Leseansicht
beziehungsweise Sternkarte aus ee85750 bei aktuellem übrigen Testaufbau.
Das endgültige entpackte Intel-Paket besteht mit 126 Desktop-, 142 Tastatur- und
75 Sicherungs-UI-Aussagen sowie Einstellungsneustart und CLI-Sicherung. SHA-256:
`c9cb71de396589fbed0b6a300d696b279f292011b872092cb8652c43241cbc13`. Das geprüfte Entwicklungspaket ist lokal unter
dist/SecondBrain auf 0.9.5 übernommen; Version und eigenes Projekt-Standbild
sind geprüft. Neue Plattformabnahme folgt nach dem Push.

Die [0.9.5-Abnahme zu 37dba58](https://github.com/Lulus792/SecondBrain/actions/runs/37556151012)
besteht inzwischen mit allen 20 Jobs einschließlich Windows Debug und vier
entpackten Paketen. Dieser Nachweis betrifft die geprüfte Version 0.9.5;
die unveröffentlichte Tabellenarbeit 0.9.6 ist noch nicht abgenommen.


## Tabellenansicht und native Struktur in 0.9.6

Am 7. Oktober ist der eigene C-Tabellenparser mit Kopf-/Trennzeile, Ausrichtung,
maskierten Pipes, fehlenden und zusätzlichen Datenzellen umgesetzt. Die
Leseansicht verwendet Spalten oder gestapelte Zeilen; die Sternkarte verwendet
nur tatsächlich dargestellte Zellverweise. Originalbytes bleiben im Editor,
in der Datei und in Sicherungen erhalten. [Vertrag und Grenzen](TABELLEN.md).

Die native Struktur enthält Tabelle, Zeilen, Spaltenköpfe/Zellen, Text und
Linkaktionen mit Indizes. Eine eigene C-Ergänzung veröffentlicht unter macOS
die bisher fehlende Zeilenliste und aktiviert sie auch auf älteren Systemen
unabhängig vom neueren Überschriftenrollensymbol. Tatsächliche Provideraufrufe
prüfen vier Zeilen mit je drei Zellen, Werte, leere Zellen und die Scrollanfrage
auf eine zunächst unsichtbare Datenzelle. Tabellenlinks öffnen sich per Tastatur.

Lokal: 18 Debug-Kernprüfungen bestehen (22,59 s); nach ergänzten Grenzfällen
bestehen drei Markdown-Nachprüfungen (2,80 s). ASan/UBSan besteht mit 5.644
Tabellenassertions einschließlich 5.000 begrenzter Eingaben, 64/65 Spalten und
65.536/65.537 Zellen. Der erste vollständige Release-Lauf besteht mit 32 Tests
(470,15 s). Nach den letzten Geometrie-/Ausrichtungskorrekturen bestehen sechs
passende Prüfungen (34,86 s), darunter 256 native Assertions; weitere 41
Graph-Assertions bestehen. Die neue Zeilenprüfung scheitert beim Rücksetzen
auf die vorherige Clip-Geometrie und besteht mit aktuellen Zeilenrechtecken.

Tatsächliche Rasterbilder wurden bei 1336×840 und bei 780×560 mit 200-%-Schrift
betrachtet, einschließlich Spaltenausrichtung, gestapelter Datenzeile nach
nativer Scrollanfrage und großer Leseansicht. UI-Nachprüfung SBUI-034–036:
/Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-07_10-19-25/UI_REVIEW.md.
Lokale Logs: build/table-core-check.log, table-core-final-check.log,
table-sanitize-final.log, table-ui-full-check.log, table-ui-final-check.log,
table-graph-final-check.log und table-regression/result.log.

Das entpackte Intel-Paket 0.9.6 besteht mit 126 Desktop-, 142 Tastatur- und
75 Sicherungsassertions, zwei Neustartprozessen mit erhaltenen Einstellungen
sowie dem produktiven Sicherungswerkzeug. Unicode-Pfade und der verschobene
Paketordner wurden tatsächlich verwendet. Archiv-SHA-256:
`4d35909f54230806131886602d7872885270d1a595a4734e4ebb936262ff64f7`.
Log: build/table-package-check.log. dist/SecondBrain enthält dieses geprüfte
Entwicklungspaket; die ehrliche lokale Buildkennung ist 37dba5897e9d-dirty,
weil es vor dem Commit gepackt wurde. Die geprüfte Umsetzung ist als
[648a696](https://github.com/Lulus792/SecondBrain/commit/648a69631c990fdcfb27dbfabfaa3af1a24aab12)
gepusht. [Native CI-Abnahme](https://github.com/Lulus792/SecondBrain/actions/runs/37593650781)
läuft noch; neue Windows-/Linux-/ARM64-Ergebnisse werden erst nach Abschluss
übernommen. UIA GridPattern und AT-SPI Table fehlen in den festgelegten Providern.
Die vorbereitete Clientprüfung unterscheidet diese Lücke von zugänglichen
Zeilenkindern. Menschliche Screenreader-Abnahme, volle GFM-Containerregeln und
Leistung sehr großer Tabellen bleiben offen. Die gestapelte Kopfzeile soll bei
wenig Höhe weiter verfeinert werden. Dies ist keine vollständige 1.0-Abnahme.


## Schmale Tabellen und native Prüflaufkosten in 0.9.7

Bei 200-%-Schrift im 780×560-Fenster beanspruchte eine zusätzliche gestapelte
Kopfzeile den ersten Bildausschnitt. Die Ansicht beginnt nun bei Tabellen mit
Datenzeilen direkt mit beschrifteten Werten. Logische native Spaltenköpfe und
Zellidentitäten bleiben erhalten; ihre ausgeblendete Zeile hat kein sichtbares
Rechteck. Kopfzeilen mit Links und Tabellen ohne Datenzeilen bleiben sichtbar.
Leere Beschriftungen erzeugen keine zusätzliche Leerzeile.

Fünf passende lokale UI-/Kernprüfungen bestehen (34,07 s); abschließend bestehen
270 native Assertions (26,67 s), einschließlich unveränderter Tabellenstruktur,
sofort sichtbarer erster Datenzelle, Scrollanfrage und Tab/Enter auf einem
Kopfzeilenlink. Rasterbilder wurden betrachtet. Die neue Prüfung scheitert mit
dem 0.9.6-Renderer an der weiterhin sichtbaren Kopfzeile. Originalbytes bleiben
erhalten. Logs: build/compact-table-final-check.log, compact-table-pump-check.log
und compact-table-regression/result.log.

Der [0.9.6-Windows-Debug-Job](https://github.com/Lulus792/SecondBrain/actions/runs/37593650781/job/112700888022)
endet bei native-accessibility nach 120 s mit Timeout; die übrigen 30 Tests
bestehen. Dieser Lauf ist deshalb nicht vollständig grün. Das Prüfprogramm
berechnet bei Clientpolling und den Zwischenphasen einer Scrollanfrage weiterhin
Eingaben, Layout, native Veröffentlichung und Aktionen, zeichnet jedoch nur
beobachtete Endzustände. Snapshot- und Zustandsprüfungen bleiben erhalten.
Der normale Anwendungsrenderer ist unverändert. Vier Phasen-Checkpoints nennen
die erreichten Abschnitte und ihre Dauer; die CTest-Fehlerannotation bewahrt sie
auch bei langen Logs. Das wurde mit einem gezielt fehlschlagenden lokalen
Diagnosefall geprüft. Die Ursache und Behebung des Windows-Timeouts werden erst
nach einem neuen nativen Lauf als bestätigt geführt; kein Geräte-FPS-Nachweis.

Das entpackte Intel-Paket 0.9.7 besteht mit 126 Desktop-, 142 Tastatur- und
75 Sicherungsassertions, zwei Einstellungs-Neustartprozessen und dem produktiven
Sicherungswerkzeug. Archiv-SHA-256: `81da4693a2efdb816f782c8f8dd4710fb75643ec62bd477664108bc6da5e31de`.
Log: build/compact-table-package-check.log. dist/SecondBrain enthält dieses
geprüfte Entwicklungspaket; Buildkennung 70c702bb89d8-dirty, vor Commit gepackt.
Die Umsetzung ist als 2edb2bf gepusht; die
[native 0.9.7-Abnahme](https://github.com/Lulus792/SecondBrain/actions/runs/37596496154)
läuft noch. Neue Plattformnachweise stehen bis zum tatsächlichen Abschluss aus. Native Matrixschnittstellen, volle
Containerregeln und menschliche assistive Bedienung bleiben Release-Aufgaben.


Der 0.9.6-Lauf 37593650781 ist inzwischen abgeschlossen: 19 von 20 Jobs bestehen,
einschließlich aller vier entpackten Release-Pakete. Einziger Fehler bleibt
Windows Debug / native-accessibility / Timeout 120s. Die erfolgreichen Pakete
belegen ihren konkreten Release-Ablauf, ersetzen aber keinen grünen Debug-Job.

UI-Nachprüfung SBUI-037: /Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-07_10-49-33/UI_REVIEW.md.


Nachprüfung zu 2edb2bf am 7. Oktober: Der neue native Lauf 37596496154 besteht
inzwischen für Windows und Linux in Debug und Release, einschließlich der
nativen Prüfung und der jeweiligen entpackten Release-Pakete. Windows Debug
ist damit nach dem vorherigen nativen Timeout wieder vollständig erfolgreich.
Die noch laufenden/ausstehenden Mac-Jobs werden separat nach Abschluss bewertet.


## Unicode-Zeichengrenzen und Editorläufe in 0.9.8

Am 7. Oktober werden Unicode 18.0.0 und UAX #29 Revision 49 als festgelegte Grundlage
für eigene erweiterte Graphemgrenzen verwendet. Eigener C-Code und abgeleitete
Eigenschaftstabellen behandeln Akzente, Flaggen, Emoji-Verbindungen, Hangul und
indische Verbindungszeichen. Alle Nuklear-Eingaben benutzen dieselben Hooks für
Cursor, Auswahl und Löschen. Intern bleiben Skalarindizes erhalten. Ein
Texteingabeereignis wird atomisch und als ein Undo-Vorgang verarbeitet; auch
Überschreiben ersetzt vollständige Grapheme. Native Editor-Teilmarkierungen
werden auf vollständige Zeichen erweitert. [Vertrag und Quellen](GRAPHEME.md).

Die Rasterprüfung deckte unvollständige Editorzeilen auf. Addierte Einzelglyph-
breiten passten nicht zum geformten Textlauf; die Ausgabe wurde dadurch gekürzt.
Die Editorzeilen werden nun als vollständige Läufe gemessen und geometrisch
begrenzt. Kommandoprüfungen verlangen vollständige UTF-8-Zeilen für Hangul,
Devanagari, Emoji, Ligatur und Akzent. Emoji-Fontabdeckung, komplexe Bidi-/Maus-
geometrie, IME-Position und native Zeichenrechtecke bleiben eigene Release-Aufgaben.

19 Debug-Kernprüfungen bestehen (35,13 s). Der Segmentierer besteht 18.764 Assertions
gegen alle 853 offiziellen Unicode 18-Fälle, ungültige/leere Eingaben und eine
100.001 Byte lange Kombination. ASan/UBSan besteht. Datenerzeugung reproduziert
bytegleich die festgelegte C-Tabelle. Erster vollständiger Release-Lauf: 33 Tests
bestehen (270,67 s). Nach den letzten Raster-/Pfadtestkorrekturen bestehen sechs
passende Tests (100,34 s), darunter 111 Editor-, 273 native und 75 Sicherungsassertions.
Die eigene UI-Implementierung wird ebenfalls unter ASan/UBSan nachgeprüft; UI-
Abhängigkeiten sind dabei nicht vollständig instrumentiert. Ohne die neuen
Graphemhaken scheitert die Cursorregression am Akzent. Logs: build/grapheme-core-
full-check.log, grapheme-sanitize.log, grapheme-ui-full-check.log, grapheme-final-
check.log, grapheme-regression/result.log und grapheme-ui-sanitize/final.log.

Die 18 Lizenzressourcen bestehen lokal, einschließlich des mitgelieferten Unicode-
Originals. Ein früherer Paketdurchlauf scheitert durch eine zwischenzeitlich
geänderte Systemzwischenablage während einer Testpfadeingabe. Pfadfixtures der
Sicherungsprüfung werden nun über echte SDL-Texteingaben gesetzt; Kopieren wird
weiter geprüft. Dieser fehlgeschlagene Lauf bleibt als Fehler dokumentiert.
Das abschließende Paket wird neu erzeugt und entpackt geprüft; dist/SecondBrain
bleibt bis dahin bei 0.9.7. Neue native Plattformabnahme folgt nach Push.

Die [0.9.7-Abnahme zu 2edb2bf](https://github.com/Lulus792/SecondBrain/actions/runs/37596496154)
steht inzwischen mit allen 20 Jobs und vier entpackten Paketen. Das ist der
Nachweis der vorherigen Version, keine Abnahme der noch unveröffentlichten 0.9.8.


Ein weiterer 0.9.8-Paketversuch scheitert im Lizenz-Kopiertest nach zusätzlichen
Zeichenframes. Testeingaben sind nun vom Systemclipboard entkoppelt und die
Kopierergebnisse werden direkt nach der tatsächlichen SDL-Aktion gelesen.
Desktop-/Tastaturnachprüfung und abschließende Paketwiederholung folgen.
UI-Bericht SBUI-038–040: /Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-07_11-50-23/UI_REVIEW.md.


Die letzten Testeingabe-Anpassungen bestehen im Desktopablauf mit 126 Assertions
und nach UTF-8-sicherer Abschnittszufuhr im Tastaturablauf mit 142 Assertions
(53,65 s). Kopierprüfungen lesen echte SDL-Zwischenablagewerte unmittelbar nach
der Aktion. Lange Fixtures werden in vollständigen UTF-8-Abschnitten innerhalb
des vorhandenen Ereignislimits zugeführt; zuvor fehlte bei einer Einzelzufuhr
der Rest eines langen Linkfixtures. Die normalen App-Eingaben und Kopieraktionen
bleiben unverändert. Das abschließende Paket wird erneut erzeugt.


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


## Schriftwahl und Emoji in 0.9.9

Am 7. Oktober ergänzt eine unveränderte, festgelegte Noto-Emoji-Schrift die UI.
Eigener C-Code wählt Ersatzschriften für vollständige Grapheme, formt benachbarte
kompatible Zeichen und verwendet dieselbe Auswahl für Messung und Rasterung.
Die Grundlinie wird aus den Schriftmetriken zusammengesetzt; Alpha wird beim
Zusammenfügen erhalten und erst bei der Anzeige verrechnet. Die App bietet jetzt
19 Original-Lizenzressourcen. [Quellen, Hashes und Grenzen](EMOJI.md).

Der erste gesamte lokale Release-Lauf besteht mit 33 Tests (620,10 s); er läuft
auf einem gleichzeitig belasteten Rechner und ist kein Geräte-Leistungsnachweis.
Nach abschließender Fehlerbehandlung/Alpha-Korrektur bestehen drei passende Tests
(6,61 s), darunter 43 Text-, 111 Editor- und 58 Lizenzassertions. Eigene Text- und
Graphemimplementierung besteht unter ASan/UBSan; UI-Abhängigkeiten sind nicht
vollständig instrumentiert. Der vorherige Renderer mit lediglich ergänzter
Emoji-Schrift scheitert an der verbundenen Frau/Laptop-Breite (33 statt 16,5).
Rasterprüfung zeigt verbundene Familie, Flagge, Hautton und gemischte Hangul-/
Devanagari-Zeilen. Logs: build/emoji-ui-full-check.log, emoji-final-render-check.log,
emoji-regression/result.log und emoji-sanitize/result.log.

Die neue Paket- und native Plattformabnahme folgt separat. dist/SecondBrain
bleibt bis zum bestandenen neuen Paket bei 0.9.8. Bidi-Absätze, präzise visuelle/
native Textgeometrie, IME und menschliche assistive Bedienung bleiben offen.
UI-Nachprüfung SBUI-040: /Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-07_12-29-00/UI_REVIEW.md.


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


## Support und nachvollziehbare Kernfehler

Am 7. Oktober sind öffentliche Fehler-/Vorschlagsformulare, SUPPORT.md,
CONTRIBUTING.md, PR-Vorlage und ein dokumentierter UI-Abhängigkeitsupdate-Ablauf
vorbereitet. Die drei Formular-/Konfigurationsdateien sind mit Psych als YAML
geprüft; lokale Dokumentverweise sind aufgelöst. GitHub-Serverdarstellung folgt
nach Veröffentlichung. Das eigene C-Programm und das geprüfte Paket bleiben unverändert.
Vertraulicher Sicherheitskanal ist angefragt; eine Vorlage aktiviert ihn nicht.
Vollständige transitive Lizenzzuordnung und Wartungsverantwortlichkeit bleiben offen.

Der [0.9.9-Nachlauf zu 1b17738](https://github.com/Lulus792/SecondBrain/actions/runs/37608867987)
findet zusätzlich einen Windows-Debug-Kernfehler im CTest-Schritt. Die bisherige
Annotation enthält nur Exit 1; konkrete Ursache ist noch nicht festgestellt.
Der Kernworkflow benutzt jetzt wie die UI den vorhandenen CTest-Wrapper, veröffentlicht
passende Fehlerblöcke und bewahrt LastTest.log/checked-ctest.log als Artefakt.
Ein absichtlich fehlschlagender lokaler CTest bestätigt erhaltenen Fehlerstatus,
Ursachen-Annotation und Prozent-Escaping; Log unter build/core-diagnostic-proof.
Die neue YAML-Workflowstruktur ist gelesen und geprüft. Dies ist Diagnoseverbesserung,
keine behauptete Behebung des unbekannten Windows-Fehlers; neuer nativer Lauf folgt.

Nach Veröffentlichung als 099de24 sind die neuen Formulare im Repository.
Der Prüftab für GitHubs Formularauswahl wird auf die Anmeldung umgeleitet;
die tatsächliche Serverdarstellung ist damit noch nicht abgenommen. Es wurde
kein Testissue versendet und keine Anmeldung vorgenommen. Die neue native
Abnahme 37609566084 ist gestartet; konkrete Ergebnisse folgen separat.


## Abschnittstrennungen und Reader-Abstände in 0.9.10

Die eigene C-Blockerkennung veröffentlicht Abschnittstrennungen als eigenen Typ.
Die Leseansicht zeichnet eine waagerechte Linie und veröffentlicht horizontale
native Separatorsemantik, ohne zusätzlichen Tabstopp. Setext und Code behalten
ihre Bedeutung; gespeicherte Originalbytes bleiben erhalten. Bei 200 Prozent
Schriftgröße werden Reader-Innenabstände mit skaliert, damit der innere
Kontrastring Abstand zum Inhalt hält. Der fast unsichtbare Hilfe-Icon-Punkt
wird als Kreisfläche gezeichnet. [Quellen und Vertrag](TRENNLINIEN.md).

Drei erste gezielte Prüfungen bestehen (15,31 s), darunter 334 native,
111 Editor- und 181.838 Blockassertions mit 10.000 begrenzten Eingaben. Der
vorherige Blockleser scheitert an der neuen Blocktypprüfung. Eigene Block-/Test-
Instrumentierung besteht unter ASan/UBSan; andere Kernteile sind dabei nicht
vollständig instrumentiert. Erster 33-Test-Lauf: 198,26 s. Die helle native
Kontrastnachprüfung besteht mit 340 Assertions (10,67 s). Nach abschließender
Paddingkorrektur bestehen erneut alle 33 lokalen Release-Tests (195,44 s),
einschließlich realer Linienpixel und beidseitigem Abstand zum Kontrastring.
Raster bei 100/200 Prozent sowie vor/nach Hilfe-Punkt und Kontrastpadding betrachtet.
Logs: build/rules-target-check.log, rules-regression/{result,sanitize}.log,
rules-full-check.log, rules-contrast-check.log und rules-padding-full-check.log.
UI-Bericht SBUI-041–043: /Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-07_13-01-27/UI_REVIEW.md.
Neues entpacktes Paket und neue native Plattformabnahme folgen separat.

Der [0.9.9-Lauf zu 099de24](https://github.com/Lulus792/SecondBrain/actions/runs/37609566084)
besteht inzwischen unter Windows und Linux vollständig einschließlich Debug/Release
und entpackter Release-Pakete. Windows-Debug-Kern besteht wieder; die Ursache
des früheren einzelnen Fehlers ist weiterhin unbekannt. Mac-Jobs stehen noch aus.
Das ist keine Abnahme der neuen 0.9.10. Verbleibende Markdown-Container, Bidi,
Textgeometrie, native Matrixschnittstellen und menschliche Bedienabnahme bleiben offen.

Die abschließende native Teilprüfung umfasst 342 Assertions.


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


## Gemeinsame Hervorhebungen in 0.9.11

Der eigene C-Inline-Leser erzeugt gemeinsame Text-/Stilbereiche für kursiv,
fett, kombinierte Hervorhebung und Inline-Code. Absätze, Überschriften und
Tabellen zeigen diese Stile mit passenden Schriftkopien. Gemeinsame Grundlinien,
Code in der umgebenden Größe und erhaltene Grapheme über Formatgrenzen sind
nachgeprüft. Winkel-URLs sind als Autolinks erkannt; rohe HTML-Tags bleiben
Literaltext. Originaldateien werden nicht geändert. [Vertrag](INLINE_STILE.md).

Alle 34 ersten lokalen Intel/macOS-Release-Tests bestehen (229,04 s). Nach
abschließender Graphem-Wortgruppierung bestehen die sieben betroffenen Tests
(18,36 s): 352 native, 85 Text-, 111 Editor-, 5.644 Tabellen-, 1.115.600 Inline-
und 180.619 Blockassertions sowie die 132 ausgewählten offiziellen Hervorhebungs-
fälle. Drei HTML-Fälle verwenden den dokumentierten Literalvertrag; dies ist
keine vollständige CommonMark-Konformität. Die kleine C-Testprobe ist danach
gegen Allokationsfehler abgesichert und separat erneut geprüft.
Neun Python-Strukturtests bestehen ebenfalls (0,422 s).

Eigene Inline-/Blockleser und Tests bestehen unter ASan/UBSan. Zusätzlich bestehen
85 Textassertions mit instrumentierten eigenen UI-/Text-/Graphemteilen;
externe Bibliotheken und übriger Kern sind dabei nicht vollständig instrumentiert,
Leakprüfung ist deaktiviert. Ohne Emphasis-Initialisierung scheitert die
Regressionsprüfung an INLINE FAIL 9. Drei abschließende Rasterbilder betrachtet.
Logs: build/emphasis-ui-full-check.log, emphasis-final-check.log,
emphasis-probe-final.log, emphasis-python-check.log,
emphasis-sanitize/result.log, emphasis-ui-sanitize/result.log und
emphasis-regression/result.log. UI-Bericht SBUI-044/045: /Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-07_14-30-33/UI_REVIEW.md.

Native Inline-Stilattribute, gemischte Bidi-Absätze, kontextuelle Scriptformung
über Stilgrenzen, genaue Textgeometrie, IME, volle Container-/Listenregeln und
menschliche assistive Bedienung bleiben offen. Neues Paket und neue Plattform-
abnahme folgen nach dem Quellcommit. Version 1.0 bleibt nicht freigegeben.


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


## Native Textstile in 0.9.12

Leseabsätze, formatierte Überschriften, Codeblöcke und Tabellenzellen veröffentlichen
Stilbereiche mit Gewicht, Kursivmerkmal, primärer Schriftfamilie und logischer
Größe. Sichtbare und native Bereiche verwenden dieselbe Graphem-Anpassung.
Snapshots besitzen ihre eigenen Kopien und aktualisieren auch bei Stiländerungen.
Der Quelleditor behält seine zusammenhängende unformatierte Auswahl-/Bearbeitung.
[Vertrag und Originalquellen](NATIVE_TEXTSTILE.md).

Drei erste gezielte Prüfungen bestehen (18,59 s); die erweiterte Nachprüfung
besteht mit 374 nativen, 85 Text- und 111 Editorassertions (20,21 s). Alle
34 lokalen Intel/macOS-Release-Tests bestehen (279,40 s). Zusätzliche native
Zellattributprüfung: 376 Assertions, 17,84 s. Abschließende API-Attributprüfung
bei ausschließlich geändertem Stil: 380 Assertions, 12,63 s. Der API-Test hält
Text, Rolle, Titel und Geometrie unverändert; er ist zusätzlich zur tatsächlichen
UI-Integration ausgeführt. Aktuelles Schrift-Raster betrachtet. Der vorige
Provider scheitert an fehlenden Fontattributen (ACCESSIBILITY FAIL 321).

Eigene UI-, Text-, Desktop-, Zugänglichkeits-, Graphem- und Inline-Komponenten
sowie native Tests sind unter ASan/UBSan instrumentiert; erster erweiterter Lauf
besteht mit 376 Assertions. Übriger Kern/externe Bibliotheken sind nicht
vollständig instrumentiert und Leakprüfung ist deaktiviert. Die abschließende
Sanitizer-Nachprüfung besteht ebenfalls mit 380 Assertions. Ohne Stilanteil
in der Änderungskennung scheitert die reine Stil-Fixture an ACCESSIBILITY FAIL 362.
Logs: build/native-styles-{target,final,full,cell,only}-check.log,
native-styles-sanitize/{result,final-result}.log,
native-styles-regression/result.log und native-styles-signature-regression/result.log.
UI-Bericht SBUI-046: /Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-07_14-53-36/UI_REVIEW.md.

macOS-Fontattribute sind tatsächlich über AppKit-Textbereiche gelesen; neue
Windows-UIA- und Linux-AT-SPI-Aufrufe sind ergänzt. Neue native Plattform-/Paket-
abnahme folgt nach Quellpush. Eine primäre Familie ist keine tatsächliche
Fallbackangabe pro Zeichen; grobe Laufrechtecke sind keine exakten Zeichenrechtecke.
Menschliche assistive Bedienung, Bidi/IME/Geometrie, native Tabellen-Matrix-
schnittstellen und weitere Release-Aufgaben bleiben offen. 1.0 ist nicht freigegeben.


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


## Windows-UIA-Textsuche in 0.9.13

Die tatsächliche 0.9.12-CI 37624868119 scheitert unter Windows Debug/Release
am ersten Fontattributfall; 32 andere UI-Tests dieser Systeme bestehen. Linux
Debug/Release besteht die Stilabfragen und sein Release-Paket. Festgelegte
Adapterquelle enthält FindText ohne Implementierung; HRESULT 0 mit leerem
Pointer ist ein bestätigter Befund. Kleine UI-Adapter-Ergänzung und fester
Windows-Quellbuild sind vorbereitet. [Vertrag/Quellen](UIA_TEXTSUCHE.md).

Ursprüngliche Anwendung, Wiederholung ohne Änderung, Fremdquellen-Abweisung
und unveränderter übriger Lockfile-Graph bestehen lokal; Windows-Metadaten werden
mit cargo metadata --locked aufgelöst. Vier lokale Mac-Nachprüfungen bestehen
(19,25 s), mit 380 nativen, 85 Text- und 111 Editorassertions. Keine tatsächliche
Windows-Kompilierung daraus ableiten. Die bisherigen nativen Stilprüfungen
bleiben erhalten; neun neue Windows-Suchfälle sind vorbereitet. Neue native
Kompilierung, Bedien- und DLL-Paketabnahme folgen nach Push. Lokale dist-App
bleibt bis zu einer geprüften neuen Paketfassung bei 0.9.12. UI-Bericht: /Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-07_15-23-52/UI_REVIEW.md.


Der erste Quellbuild-Lauf 37628165496 stoppt unter Windows Debug/Release schon
im Konfigurationsschritt. Die vorhandenen Annotationen enthalten nur Exit 1;
Ursache ist noch nicht belegt. Öffentliche REST-Logabfrage ist nicht verfügbar,
der veröffentlichte HTML-Logverweis liefert keinen zugänglichen Detailblock.
Ein Konfigurationswrapper erfasst jetzt beide Ausgabeströme, erhält den
Fehlerstatus und veröffentlicht die Ursache als Annotation sowie checked-configure.log.
Eine absichtlich fehlschlagende lokale Konfiguration bestätigt Status, Log und
Prozent-Escaping; YAML ist mit Psych gelesen. Neuer nativer Diagnoselauf folgt.
Dies ist Diagnoseverbesserung und keine behauptete Windows-Behebung.

Das lokale Intel-Paket 0.9.13 zu 2242b5d besteht bereits mit Desktop 126,
Tastatur 142, Sicherung 75, Neustart und CLI; Log windows-findtext-package-check.log.
Dist bleibt vorerst bei geprüftem 0.9.12. Paketprüfung ist kein Windows-Nachweis.


Der Diagnoselauf 37629012326 stoppt ebenfalls beim Windows-Konfigurationseinstieg;
keine der vorgesehenen Konfigurationsdateien wird als Artefakt gefunden. Die
Ursache ist damit weiterhin unbekannt. Der Einstieg verwendet nun ausdrücklich
Bash, vollständig zitierte absolute Quell-/Buildpfade und ein zusätzliches
Shell-Einstiegslog. Ein separater Berichtsschritt veröffentlicht dessen Inhalt,
auch wenn der innere Wrapper noch nicht erreicht wird. Prozent-/Zeilen-Escaping
ist mit der echten lokalen Fehlerdatei geprüft; Workflow-YAML ist gelesen.
Eine vollständige lokale CMake-Fixture bindet die Quellbuild-Integration ein und
konfiguriert erfolgreich; sie kompiliert keine Windows-Bibliothek.
Die Quellpatch-Prüfung akzeptiert auch eine CRLF-Fassung des eigenen Snippets;
daraus wurde kein reproduzierter Fehler abgeleitet. Neuer nativer Diagnoselauf folgt.


Der ausführliche Windows-Nachweis 37630397578 nennt nun die Ursache:
CMake schreibt das geänderte text.rs mit CRLF. Sein tatsächlicher Hash
`4c9836aa3845b12cdca68c8f7bbad0b1f6ae88d7f4ba10911a2e5881f86f798f`
ist bytegenau der CRLF-Fassung des vorgesehenen LF-Ergebnisses. Eine CRLF-Fassung
des eingelesenen eigenen Snippets war hingegen lokal kein Fehler; der relevante
Unterschied entsteht beim Windows-Dateischreiben.

Der Vorbereitungsschritt verwendet jetzt file(CONFIGURE) mit NEWLINE_STYLE UNIX.
Originalquelle, bereits vorbereitete LF-Fassung und exakt die hinterlassene
Windows-CRLF-Fassung ergeben denselben festgelegten LF-Hash. Unbekannte Quellen
werden weiterhin abgewiesen. Alle vier lokalen Quellenfälle bestehen; Logs in
build/windows-findtext-lineend-proof. Kein temporärer Konfigurationseingang bleibt
als Buildabhängigkeit zurück. Neue tatsächliche Windows-Konfiguration/Kompilierung
und native Ausführung folgen; noch keine Windows-Abnahme behaupten.


Die native Windows-Konfiguration zu e6c30d0 besteht; der Quellbuild meldet nun
zwei tatsächliche Typfehler: windows-strings 0.5.1 stellt BSTR als Deref<[u16]>
bereit, nicht mit as_wide; windows-result 0.4.1 verwendet Error::from_thread,
nicht Error::from_win32. Beide Aufrufe sind anhand der originalen, gehashten
Lockfile-Quellen korrigiert. Der neue Ergebnis-Hash ist
`453fcaaa4faf52c87cc8f15fc50d4710540e35bc181bd6ac3daf28613ed7874a`.
Original, beide exakt bekannten bisherigen Fassungen und neue LF-Fassung
werden reproduzierbar auf diese Version gebracht; unbekannte Quellen bleiben
abgewiesen. Vier Quellenfälle und Abweisung bestehen lokal. Windows-Kompilierung
und native Clientausführung werden im neuen Lauf weiter geprüft.


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


## Zeichenreferenzen und Mathematikglyphen in 0.9.14

Eigener C-Decoder verarbeitet alle 2.125 festgelegten Semikolon-Namen sowie
numerische Unicode-Angaben. Titel, Leseansicht, native Texte, Tabellen, Inline-
Linkziele und Sternkarten verwenden dieselbe Projektion. Code, Maskierungen,
rohe HTML-Tags und Autolinks behalten ihren Literalvertrag; Originaldateien
werden nicht verändert. Daten-/Fontlizenzen gehören zu App und Paket; die
Lizenzansicht hat 21 Originaltexte. [Vertrag und Quellen](ENTITIES.md).

Der unabhängige Datenvergleich besteht mit allen 2.125 Namen und allen Namen
innerhalb von Code. 15 ausgewählte Original-Normfälle bestehen; Referenzlink-
und Listenfälle bleiben separat offen, Roh-HTML und terminaler HTML-Code-
Zeilenumbruch folgen dem ausdrücklich beschriebenen Anzeigevertrag.
1.194.343 eigene Inlineassertions einschließlich 5.000 begrenzter Quellfälle,
expandierender Ausgabe, exaktem Zielpuffer, Ausgabegrenze und großem CRLF-Code
bestehen unter eigener ASan/UBSan-Instrumentierung.

Nach ergänzter Noto-Math-Schrift bestehen alle 36 lokalen Intel/macOS-Release-
Tests (242,73 s), darunter 393 native, 91 Text-, 111 Editor- und 62 Lizenz-
assertions. Nach präziser Kapazitätskorrektur bestehen erneut vier Kern-/Norm-
Nachprüfungen (9,91 s) und vier native/Text-/Graph-/Tabellen-Nachprüfungen
(31,03 s). Neun Python-Strukturtests bestehen (0,390 s); Datengenerator erzeugt
bytegenau dieselbe Tabelle. Vorheriger Inline-Leser scheitert an Referenzausgabe;
vorheriger Fallback an Math-Kombinationsbreite. Bilder vor/nach betrachtet;
Kombinationen werden nach der Fontkorrektur tatsächlich sichtbar.

Die gezielte eigene UI-/Text-/Desktop-/Zugänglichkeits-/Graphem-/Inline-
Instrumentierung besteht zunächst mit 393 nativen Assertions. Die letzte
native Sanitizer-Nachprüfung nach der Kapazitätskorrektur besteht ebenfalls
mit 393 Assertions. Andere
Kernteile und externe UI-Bibliotheken sind dabei nicht vollständig instrumentiert;
native Leakprüfung ist deaktiviert. Logs: build/entities-{core,math,final-full,
capacity,final-native}-check.log, entities-sanitize/final-result.log,
entities-ui-sanitize/{result,final-result}.log, entities-regression/{inline,text}.log
und entities-final-python-check.log. UI-Bericht SBUI-048/049: /Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-07_16-43-55/UI_REVIEW.md.

Neue Paket-/Plattformabnahme folgt nach Quellpush. Bidi, exakte visuelle/native
Textgeometrie, IME, Referenzlinks, volle Listen/Container, weitere Glyphenabdeckung
und menschliche assistive Bedienung bleiben offene Release-Arbeiten.
1.0 ist nicht freigegeben; vorhandene Arbeitskopie bleibt bis zum neuen Paketnachweis.


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


## E-Mail-Autolinks und Linkvorrang 0.9.15

Der eigene C-Leser ergänzt [E-Mail-Autolinks](AUTOLINKS.md) mit wörtlicher
Beschriftung und mailto-Zielen. Die UI kodiert automatisierte Empfänger vor
der OS-Übergabe; Sonderzeichen werden keine zusätzlichen Mailfelder. Code und
Maskierungen bleiben literal. Ein innerer Autolink verdrängt einen äußeren
Link, statt dessen falsches Ziel zu aktivieren; DEL ist aus URI-Autolinks ausgeschlossen.
Referenzlinks sind [recherchiert](REFERENZLINKS_RECHERCHE.md), noch nicht implementiert.

Alle 37 lokalen Intel/macOS-Release-Tests bestehen (236,00 s), darunter 401
native, 91 Text- und 111 Editorassertions. Fünf Vorprüfungen bestehen (21,56 s).
19 unveränderte Originalfälle vergleichen echte C-Texte/Stile/Ziele; 1.000
eigene Adressfälle bestehen gegen separaten Regex. Gezielte eigene Kern-
ASan/UBSan besteht mit 1.194.616 Assertions und 5.000 begrenzten Quellfällen;
native eigene Teilinstrumentierung mit 401 Assertions. Leakprüfung ist auf
diesem macOS nicht verfügbar und deaktiviert; übriger Kern/externe UI-Bibliotheken
sind nicht vollständig instrumentiert.

Vorheriger Parser scheitert an E-Mail-Ausgabe und liefert bei verschachteltem
URI das falsche äußere Ziel. Actual Reader-Absätze/Zellen, zwei Enter-Aktionen
mit aufgezeichnetem URL-Adapter, unveränderte Quelle und Raster sind geprüft.
Es wurde keine tatsächliche Mail-App gestartet oder bedient. Logs:
build/autolinks-{targeted,full,oracle}-check.log, autolinks-sanitize/result.log,
autolinks-ui-sanitize/result.log und autolinks-regression/{result,nested}.log.
UI-Bericht SBUI-050/051: /Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-07_17-08-19/UI_REVIEW.md.
Neue Paket-/Plattformabnahme folgt nach dem Quellpush; dist bleibt bis dahin
beim geprüften 0.9.14. Der vollständige Auftrag bleibt aktiv, 1.0 unverändert offen.


## Lokal installiertes Paket 0.9.15; GitHub-Push ausstehend

Das Intel/macOS-Paket zu 9e6cd56 besteht mit 126 Desktop-, 142 Tastatur- und
75 Sicherungsassertions, zwei isolierten Einstellungs-Neustartprozessen sowie
der produktiven Sicherungs-/Prüf-/Wiederherstellungs-CLI. App und CLI melden
0.9.15, Build 9e6cd5675f78. Archiv-SHA-256:
`baeb9831bf752a095d644588f817ec48429922ba5cb42dbc19aa734644ca41f1`.
Log: build/autolinks-package-check.log. dist/SecondBrain enthält dieses geprüfte
Paket. Vorherige 0.9.14-App bleibt unter /Users/lulus/Projects/SecondBrain/build/autolinks-previous-dist-20261007-171235. Eigenes Projektgedächtnis
gestartet, Raster erzeugt und dist/SecondBrain/preview.png betrachtet.

Vier normale SSH-Pushes wurden am 7. Oktober zwischen 17:08 und 17:12 Europe/
Berlin vom GitHub-Server mit Internal Server Error abgewiesen. Git ls-remote
bestätigte weiterhin a57bdfc auf main; 9e6cd56 ist lokal erhalten. Es wurde
keine Historie umgeschrieben. Neue Windows-/Linux-/ARM64-CI für 0.9.15 fehlt
daher noch; frühere Abnahmen werden nicht als Nachweis dieser Änderung ausgegeben.
Nächster Veröffentlichungsschritt ist ein normaler erneuter Push und die
tatsächliche Plattformnachprüfung. Der vollständige Auftrag bleibt aktiv.


## Referenzlinks und Scrollrand in 0.9.16

[Gemeinsame Referenzumgebung](REFERENZLINKS.md) in eigenem C: Definitionen aus
der ganzen Datei, erste passende Definition, volle Standard-Unicode-Faltung,
Titel, Absätze, Tabellen, native Inhalte und Graph. Originaldateien bleiben
unverändert. Code/Maskierungen/Inline-Vorrang bleiben erhalten. Weiche Umbruch-
Ränder sind bereinigt; Containerdefinitionen bleiben gesondert offen.

Der ursprüngliche Gesamtlauf besteht mit 40 Release-Tests (321,77 s). Das
betrachtete Raster bestätigt danach überlagerte Link-/Fokusränder durch die
Scrollleiste. Ein vor dem Layout reservierter Rand in Reader, Modalinhalt und
Einstieg trennt Inhalt und Pointer-Schiene; Umbruchbreite bleibt stabil. Fünf
gezielte Nachprüfungen bestehen (22,94 s); abschließend bestehen alle 40
Release-Tests nach Randkorrektur (232,30 s), darunter 433 native, 60 Graph-,
91 Text- und 111 Editorassertions. Vor-/Nach-Raster betrachtet, Abstand bei
100/200 Prozent Schrift nachgeprüft.

6.030 eigene Referenz- und 1.187.526 Inlineassertions, 81 unveränderte
Normfälle und alle 1.606 Unicode-C-/F-Mappings bestehen unter eigener
ASan/UBSan-Instrumentierung von references/markdown/inline/sb. Enthalten:
3.000 begrenzte Definitions-/5.000 Inlinefälle, erhaltene Quellen, Unicode-
Expansion, Puffer, Zahlen-/Namens-/Arbeitsgrenzen. Native Teilinstrumentierung
besteht mit 433 Assertions nach Layoutkorrektur. Leakprüfung ist auf diesem
macOS nicht verfügbar und deaktiviert; übrige Plattformteile und externe UI-
Bibliotheken sind nicht vollständig instrumentiert. Vorheriger vollständiger
0.9.15-Quellbaum scheitert an 69 der 81 Originalfälle. Roh-HTML/Bildalternativen
und terminale Code-Serializer-Zeilenumbrüche folgen dem ausdrücklich beschriebenen
Produktvertrag; keine vollständige CommonMark-Abnahme.

Code und Daten verwenden keine neue externe Fachbibliothek. Unicode-Original
und abgeleitete C-Tabelle sind unter der vorhandenen Unicode License V3
zugeordnet, Hash/Generator bytegenau geprüft. Der normale Build benötigt
keinen Datengenerator. Logs: build/references-{targeted,gutter,full,final-full}-
check.log, references-sanitize/result.log, references-ui-sanitize/result.log
und references-regression/result.log. UI-Bericht SBUI-052/053: /Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-07_17-42-40/UI_REVIEW.md.

Neue Paket-/Plattformabnahme folgt nach dem Quellcommit. GitHub-Push des
vorherigen 0.9.15 scheitert weiterhin serverseitig; aus älterer CI keine
Abnahme dieser Änderung ableiten. Dist enthält bis zum neuen Paket weiterhin
0.9.15. Vollständige Container/Definitionen darin, Bidi/Textgeometrie/IME,
Tabellenmatrix, menschliche assistive Bedienung und weitere Release-Arbeiten
bleiben offen. Der Gesamtauftrag bleibt aktiv, 1.0 bleibt unverändert offen.


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


## Voraussetzung für Container: eigene Quellprojektion

Eigener C-Baustein src/projection.c/h trennt geliehene Originalbytes und besessene
Parseransicht. Physische Tabspalten und teilweise entfernte Tabs bleiben korrekt
zugeordnet; CR/LF/CRLF und ein terminaler Parserumbruch behalten ihre Original-
positionen. Quellsuche nutzt geordnete Bereiche, benachbarte Rohkopien werden
zusammengeführt. Ungültige Bereiche, freigegebene Ansichten, Überlauf und
fehlgeschlagene mehrteilige Anfügungen werden ohne partielle sichtbare Änderung
behandelt. [Vertrag und folgende Integration](CONTAINER_PLAN.md).

Die erste Kern-Debug-Abnahme besteht mit 27 Tests (18,66 s). Nach ergänzten
Lebensdauer-/Endgrenzen bestehen die letzte Projektionsprüfung mit 315.754
Assertions (1,04 s) und gezielte eigene ASan/UBSan mit derselben Zahl. Enthalten:
alle vier Tabstartspalten, vollständige/teilweise/zu kurze Einrückung, Unicode-
Bytepositionen, 3.000 erzeugte Ansichten, erhaltene Originalbytes, 65.536
Zuordnungsbereiche, exakte 16-MiB-Endgrenze und Rücknahme nach einer bereits
erfolgreichen Teilanfügung. Nur Projektionscode/Test sind instrumentiert;
Leakprüfung ist auf diesem macOS nicht verfügbar und deaktiviert. Logs:
build/projection-{core,final}-check.log und projection-sanitize/result.log.

Dieser Baustein wird noch nicht von der App benutzt. Keine fertige Listen-/Zitat-
Darstellung oder native Containersemantik daraus ableiten; kein neues Paket
und keine andere App-Version deswegen ausgeben. Nächste Arbeit ist der
Dokumentbaum samt vollständiger gemeinsamer Integration. Gesamtauftrag und
1.0-Aufgaben bleiben offen.


## Eigener Dokumentbaum: geprüfter Kern, App-Anbindung noch offen

[Dokumentbaum](DOKUMENTBAUM.md) in eigenem C baut Container/Blätter mit
Eltern-/Geschwisterbeziehungen und besessenen Quellprojektionen auf. Physische
Tabspalten, erlaubte Fortsetzungen, leere/kompakte/lockere Listen, Code-/
Überschriften-/Trennungsregeln und Definitionen innerhalb von Listen/Zitaten
sind berücksichtigt. Referenzen leihen ihre tatsächlichen Blatttexte; erste
Definition und Unicode-Vergleich bleiben dieselben Regeln. ATX-Überschriften
erzeugen keine Definitionen; Setext prüft Definitionen vor der Umwandlung.

Alle 29 lokalen Kern-Debug-Tests bestehen (16,35 s). Nach zusätzlich geprüften
Knoten-/16-MiB-Endgrenzen bestehen zwei letzte Nachprüfungen (1,13 s); der
abschließende CTest-Wrapper besteht nochmals mit 29 Tests (23,12 s).
683.008 eigene Dokumentassertions einschließlich 4.000 begrenzter Eingaben,
Quellen-/Baum-/Positionsinvarianten, Definitionen und Grenzen bestehen unter
gezielter eigener ASan/UBSan. 307 unveränderte Normfälle bestehen mit dem
unabhängigen HTML-Baumvergleich. Prüfumfang und Literal-Ausnahmen stehen im
Vertrag; keine vollständige CommonMark-/GFM-/UI-Abnahme. Logdateien:
build/document-{targeted,core,final-targeted}-check.log, document-combined-
check.log, document-sanitize/{result,final-result}.log und document-wrapper-check.log.

Der neue Baum wird noch nicht von der App verwendet. Titel-/Graph-/Reader-/
Tabellenanbindung, native Container und sichtbare Gestaltung bleiben die
unmittelbar folgende Arbeit. Dist bleibt beim geprüften 0.9.16, Build b19b47624512.
Der vollständige Auftrag bleibt aktiv; 1.0 bleibt offen.

Im [vorherigen Lauf 37649752627](https://github.com/Lulus792/SecondBrain/actions/runs/37649752627)
zu 0a1e872 bestehen elf Windows-/Linux-Jobs; Windows Desktop Debug scheitert
nach sechs Minuten Testschritt, ohne öffentlich verfügbare LastTest-Diagnose.
Der konkrete Testfehler ist noch nicht nachgewiesen. Neuer Diagnosepfad
zeichnet den gesamten Testeinstieg über Bash/tee mit absoluten Pfaden auf,
publiziert ihn gesondert und weist leere Testsammlungen als Fehler ab.
Lokale Erfolgs-/Fehler-/Leerfall- und frühe Log-/Escaping-Prüfungen bestehen.
Das bereitet die tatsächliche Nachprüfung vor, behauptet aber keine Behebung
der noch unbekannten Windows-Testursache. Neue CI nach Push übernehmen.

## 0.9.17: gemeinsame Container in Titel, Reader und Sternkarte

7. Oktober 2026. `src/document.c` ist jetzt mit Titelermittlung, Leseansicht,
Tabellen und Graph verbunden. Definitionen aus Listen/Zitaten gelten für alle
vier Wege. Code/HTML bleiben literal; Originalbytes bleiben in Editor und Datei.
Tabellen-/Zeilenkennungen und Überschriften verwenden ursprüngliche Positionen.
Listenmarker/Nummern, Zitatlinie und native Container gliedern die bestehende
Glaskarte. Lange Nummern reservieren gemessene Breite; Marker stehen nativ vor
ihrem Inhalt. Containeranfragen verwenden den vorhandenen Scrollweg.

Parsergrenzen lassen gültige Notizen über ihren Dateinamen erreichbar. Der Reader
zeigt Originaltext ohne Linkaktionen; ein gescheiterter Graph-Neuaufbau ersetzt
keinen gültigen Graph. Neue Regressionen prüfen verschachtelte Referenzen/Tabellen,
HTML-/Code-Ausschluss, 65 Präfixe, Quellkennungen, Tastatur, 780×640-Fenster und
200-Prozent-Schrift. [Entwurf und Grenzen](CONTAINER_UI.md), UI-Bericht SBUI-054–056.

Erster vollständiger lokaler Release-Lauf: 43/43 Tests in 298,53 s.
Nach den Schlusskorrekturen: 29/29 Kern-Debug-Tests in 29,08 s; abschließender Release-Gesamtlauf: 43/43 in 271,45 s. Gezielte Graph-ASan/UBSan: 75 Assertions. Gezielte eigene native ASan/UBSan besteht mit 552 Assertions; externe
UI-Bibliotheken sind nicht vollständig instrumentiert, Leakprüfung ist auf diesem
macOS nicht verfügbar und deaktiviert. Neue Paket-/Plattformnachweise folgen;
installierte App bleibt 0.9.16.
Der zuvor ausstehende a45d7e2-Push ist erfolgreich veröffentlicht,
[Lauf 37656257074](https://github.com/Lulus792/SecondBrain/actions/runs/37656257074)
besteht inzwischen in allen zwölf Windows-/Linux-Jobs, einschließlich Desktop
Debug/Release und entpackter Pakete. Die acht macOS-Jobs warten noch. Der frühere
Windows-Debug-Ausfall tritt in diesem Lauf nicht auf; seine Ursache wurde damit
nicht bestimmt. Das ist eine Kernbaustein-Abnahme zu a45d7e2, keine neue
Plattformabnahme der noch unveröffentlichten App-Integration.

## 0.9.18: Reaktionszeit, Dialogformen und Hinweise

7. Oktober 2026. Konkretes Nutzerfeedback mit Screenshot führt zu
[erneuter Interaktionspolitur](INTERAKTION.md): gemeinsames Framebudget, früher
Beginn, kurze unterbrechbare Kartenüberblendung, fraktionales Scrollen und
unmittelbares Ziehen. Dialogmaße folgen tatsächlichem Inhalt; Zeilen haben
skalierende Innenränder. Hinweise sind blickdicht, im Fenster begrenzt, mit
eigener Kürzelplakette und ohne Eingabefenster. Die Suchleiste teilt ihre Fläche
mit einer dezenten Löschaktion; ein leeres Feld hat keine unnötige Löschung.

Erster vollständiger Release-Lauf: 44/44 in 292,10 s. Nach stärkerer Snapshot-/
Wiederholungsprüfung und Hinweisänderungen fanden Nachprüfungen Transparenz-
und Fokusfehler. Der endgültige Hinweis benutzt die vorhandene Nuklear-Overlay-
Zeichnung und erzeugt keine eigene Eingabefläche. Der abschließende 44er-Lauf
besteht in 289,12 s. Die zusätzliche Sanitizerprüfung fand danach eine Abfrage
einer noch nicht angelegten Readerzeile und einen überlaufenden Probezeiger
im UI-Puffer vor dessen Wachstum. Beide sind korrigiert; gezielte ASan/UBSan-Nachprüfung besteht mit 128 Assertions
ohne Diagnose. Leakprüfung auf diesem macOS deaktiviert, weitere externe UI-
Bibliotheken nicht vollständig instrumentiert. Endgültige Gesamt-Nachprüfung:
44/44 in 287,66 s. Alle 395 lokalen Markdown-Verweise sind erreichbar.
48 vollständige Materialraster stimmen exakt mit dem bisherigen Renderer
überein. Lokale Metal-Messungen belegen keine pauschale 60-fps-Abnahme.

Neue Intel/macOS-Paketabnahme zu **ee71cc4c74aa** besteht: 126 Desktop-,
142 Tastatur- und 75 Sicherungsassertions, zwei isolierte Neustartprozesse und
CLI-Sichern/Prüfen/Wiederherstellen. App und CLI identifizieren sauber 0.9.18.
Archiv SHA-256: `1a0cf18ca59d2352ae38cac06607aaa200bbe99abedb5ec331238bee4ebf5db7`.
Die geprüfte App ist nach dist/SecondBrain installiert; eigenes Gedächtnis
gelangt unverändert in die Leseansicht, Raster betrachtet. Rückfallkopie:
`build/previous-dist-0.9.16-20261007-212254`. Paket-/Installationslogs:
`build/interaction-package-check.log`, `build/interaction-installed-brain.log`.

Quellschritt ist nach origin gepusht. [CI 37673520042](https://github.com/Lulus792/SecondBrain/actions/runs/37673520042)
war zuletzt queued; neue Windows-/Linux-/ARM64-Nachweise sind noch offen.
UI-Bericht SBUI-057–061, Pfad in
`build/interaction-review-path.txt`; keine menschliche assistive Abnahme.

Präzisierung zum vorherigen Graph-Nachweis: Die C-Graphfunktion erhält ihren
letzten gültigen Wert bei Fehlern. Der Desktop verwirft derzeit die Darstellung,
da seine Labels/Aktionen aktuelle Notizindizes benutzen. Eine an stabile
Dokumentkennungen gebundene vorherige Desktop-Sicht bleibt Release-Arbeit.


## 0.9.19: zusammenhängender letzter Sternkartenstand

Die Desktop-Sicht hält ab 0.9.19 eine eigene Kopie von Dokumentpfaden, Titeln,
Bereichen und stabilen Kennungen neben dem Graph. Fehler eines Neuaufbaus
ersetzen keinen Teil dieses Stands; Projektgrenzen verwerfen ihn ausdrücklich.
Rendern, Kamera, native Aktionen, Maus und Tastatur verwenden diese Inventur.
[Vertrag und Grenzen](STERNKARTEN_BESTAND.md). Gezielte Fehler-/Wiederherstellungs-
prüfung besteht; neue Gesamt- und Sanitizerabnahme laufen. Dist bleibt beim
geprüften 0.9.18, bis das neue Paket geprüft ist.

[CI 37673520042](https://github.com/Lulus792/SecondBrain/actions/runs/37673520042)
zu ee71cc4: alle acht Desktop-Debug/Release-Jobs auf Windows, Linux,
macOS ARM64 und Intel einschließlich der entpackten Pakete tatsächlich bestanden.
Das sind Nachweise zu 0.9.18, keine Plattformabnahme der neuen Graphänderung.


Neues Nutzerfeedback erweitert 0.9.19 um [lange Dokumentwechsel, Suche,
Texteingabe und Anlegekarten](NAVIGATION_POLITUR.md). Erste lokale Metal-Probe
zeigt für das 54-KiB-Journal rund 95 ms Layout pro Bild. Exakte begrenzte Messungs-
und Umbruchplancaches reduzieren die warme lokale Layoutzeit auf rund 9 ms.
48 unabhängige gecachte/ungecachte Rasterfälle stimmen samt Höhen überein;
Unicode-/Schriftprüfung besteht. Such-/Cursor-/Startprüfung besteht gezielt mit
24 Assertions. Suche erhält beim Tabweg zu Ergebnissen ihre Fläche; Verlassen
in Dokument/Sternkarte oder andere Aufgaben stellt die vorherige Fläche wieder her.
Gesamtprüfung: 46/46 in 424,71 s. Nach letzter Suchfokuszeichnung und
Projektwechselkorrektur bestehen Interaktion, Graphinventur, Cache-/Navigation
und der vollständige Tastaturweg nochmals (4/4 in 155,29 s).
Endgültige Prüfungen: 95 Navigationsassertions mit 48 gleichen vollständigen
Rasterprüfsummen/Höhen, 34 Graphinventurassertions. Eigene gezielte ASan/UBSan
mit instrumentiertem Desktop, UI, Text- und Materialrenderer sowie Graph besteht
mit denselben 95 und 34 Assertions ohne Diagnose. Weitere externe UI-Bibliotheken
nicht vollständig instrumentiert, macOS-Leakprüfung deaktiviert. Paket folgt.


Intel/macOS-Paket **0.9.19, Build 9ab187aec5f2** aus sauberem Commit besteht:
126 Desktop-, 143 Tastatur- und 75 Sicherungsassertions, zwei Neustartprozesse
und produktive CLI-Sicherung/Wiederherstellung aus entpacktem Unicode-Pfad.
Versionstests 2/2. Archiv-SHA-256: `f48d59755aa637f83a20564d65f4fb80000d3133260c396ec85cf0d82ce01717`.
Geprüfte App nach dist/SecondBrain installiert, eigenes Gedächtnis geladen und
Raster mit direktem Sternkartenfokus betrachtet. Rückfallkopie: `build/previous-dist-0.9.18-20261007-222210`.
Logs: build/navigation-{package-check,install,installed-brain}.log.
Quellschritt 9ab187a ist nach origin gepusht;
[CI 37680520037](https://github.com/Lulus792/SecondBrain/actions/runs/37680520037)
besteht in macOS ARM64/Intel Debug/Release einschließlich Paketen und Windows-
Debug. Windows-Release scheitert im UI-Testschritt; beide Linux-Jobs sind
abgebrochen. Öffentlich fehlen Testdetails (Logs/Artefakte erfordern Anmeldung).
Eine Ursache ist noch nicht belegt. Der nächste Lauf verwendet einen nativen
Python-Teststarter mit früh geschriebenem Eintragslog und unverändertem CTest-
Prüfauftrag. Vier echte CMake-/CTest-Prozessfälle bestehen lokal: Erfolg,
Testfehler, keine Tests und fehlendes Verzeichnis, einschließlich Unicode/Leerzeichen.
Das ist Diagnoseverbesserung, keine bestätigte Windows-Fehlerbehebung.


## 0.9.20: UI-Lizenzinventur und mitgelieferte Originale

Festgelegte Cargo-Abhängigkeiten sind gegen die durch SHA-256 geprüfte originale
AccessKit-C-Lockdatei abgeglichen: 113 Komponenten, Mac 20 (ARM64/Intel identisch),
Windows 24, Linux 90. 57 Original-Payloads sowie Quellenhinweise aus SDL3 und
HarfBuzz sind gesammelt; darunter zuvor fehlende YUV-/HIDAPI-BSD-Hinweise.
[Nachweis und offene Laufzeitanteile](LIZENZ_INVENTUR.md).

23 Ressourceneinträge bestehen mit 66 Assertions. Zunächst 47/47 CTests in
259,02 s bestanden; nach Build-Abhängigkeits- und Quellenlinkkorrektur bestehen
Ressourcen/Tastatur/Inventur/CI-Eintrag 4/4 in 133,80 s. Sieben Inventur- und
Fehlerprovokationsprüfungen bestehen. Der Tastaturweg liest/kopiert ausgewählte
Texte; Ressourcenprüfungen decken sämtliche Dateien ab. Reine Lizenzänderung
und Wiederherstellung werden beim Neubau bytegenau in App-Ressourcen übernommen.
Zusätzliche Tastaturprobe für beide großen Sammlungen besteht in 183,62 s.
20 Python-Unittests bestehen in 2,30 s. Lizenzlistenraster im kleinen Fenster
mit 150 Prozent Schrift geprüft. Paketabnahme folgt; Dist bleibt bis dahin bei
geprüftem 0.9.19. Logs: build/license-{all-check,final-targeted,keyboard-final,
python-final}.log sowie license-resource-fixed-{change,restore}.log. Die komplette
transitive Release-Abnahme bleibt wegen Rust-/Systemruntime-Abgleich offen.


## 0.9.21: Literaltexte außerhalb des sichtbaren Bereichs

Die zusätzliche Lizenzprobe deckt wiederholte Umbrucharbeit für bereits vollständig
abgeschnittene Literalzeilen auf. Nuklear berechnet diese auch ohne sichtbare
Pixel. Eigene Dokumentdarstellung überspringt ab 0.9.21 deren Zeichenarbeit,
behält aber Zeilenhöhe, Scrollumfang und native Textspans bei. Das betrifft
auch Code- und Quellansichten. Native Textinformationen, Navigation und
Tastatur bestehen 3/3 in 163,94 s; 150 Tastaturassertions. Drei vollständige
Raster (Lizenzliste, Lizenztext und kleine Tastaturansicht) sind bytegleich zum
ungekürzten Zeichenweg. Lokal bei 600 gleichen Testframes sinkt angesammelte
Layoutzeit von 81,55 auf 4,88 s; Gesamtlauf 183,60 auf 110,59 s. Dies sind
Software-Renderer-Testmessungen, keine allgemeine FPS-Zusage. Logs:
build/literal-culling-check.log und literal-culling-raster-proof.json.
Saubere Paketierung folgt.


## Windows-Zeilenenden und ergänzte Diagnosen

CI zu 3d96c29 ([37689224148](https://github.com/Lulus792/SecondBrain/actions/runs/37689224148))
besteht zuletzt im Linux-Release einschließlich Paket und macOS-ARM64-Release.
Windows-Release führt die App-UI-Prüfungen erfolgreich aus, scheitert aber in
den neuen Inventur-/Teststarter-Prüfungen. Öffentliche Annotationen zeigen
Testnummern, noch nicht alle Ursachen. Keine komplette Windows-Abnahme behauptet.

Git mit core.autocrlf=true verändert nachweislich die Bundle-Bytes ohne besondere
Attribute. Explizite LF-Attribute erhalten beide Originalhashes exakt; lokal
mit echten Git-Checkoutfiltern geprüft. Sieben Inventurprüfungen bestehen.
Die CI-Fehlerausgabe erhält frühe Python-Fehler vor langen erfolgreichen
Log-Enden; fünf echte Prozess-/Diagnoseprüfungen bestehen. Der Windows-
Teststarterfehler bleibt bis zu genauer Diagnose und neuer Abnahme offen.
