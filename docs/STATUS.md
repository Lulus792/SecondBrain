# Umsetzungsstand von SecondBrain

Stand: 6. Oktober 2026. Version 0.3.0 mit Lumen-Sternkarte, Glaskarten und
Tastaturwegen ist auf Windows, macOS und Linux geprüft. Der aktuelle
Entwicklungsschritt 0.3.1 ergänzt Einstellungen und Ordnerwahl.

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
