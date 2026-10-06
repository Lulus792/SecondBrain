# Native UI-Anbindung

Stand: 6. Oktober 2026. Der aktive Auftrag umfasst native Screenreader-Anbindung.
Die Adapter sind in Entwicklungsschritt 0.5.0 integriert. Die hier genannten
Prüfungen belegen jeweils ihren Umfang; eine vollständige Screenreader-Abnahme
steht weiterhin aus.

Die [AccessKit C-Bindings](https://github.com/AccessKit/accesskit-c) bieten Adapter
für macOS, Windows und Unix. Das [SDL-Beispiel](https://github.com/AccessKit/accesskit-c/blob/0.23.1/examples/sdl/hello_world.c)
zeigt Aktivierung, Fokus und Aktionen. Der geprüfte Release 0.23.1 enthält C-Header
und statische UI-Bibliotheken für die benötigten Architekturen. Der lokale Download
stimmt mit dem GitHub-Asset-Digest überein:

`35b7ca8a6f1e038b5da35e1e9e5a0adaed9bfcf21e1496d29598fbbadcc7043f`

Der Einsatz ist ausschließlich auf die UI-Anbindung begrenzt. Fachlicher Kern,
Speicherung, Suche und Sicherung bleiben eigene C-Implementierungen. Die
Bibliothek ist intern in Rust geschrieben; der Anwendungscode verwendet ihre
C-Schnittstelle. Upstream nennt MIT/Apache und BSD-Anteile. Die Original-Lizenzen
und Autoren sind übernommen und werden mit den Paketen ausgeliefert.

## Umsetzung und Abnahme

- Stabile, beschriftete UI-Knoten für Fenster, Aktionen, Eingabefelder und Inhalte.
- Rollen, Werte, Fokus, Sichtbarkeit und Geometrie entsprechen der wirklichen App.
- Native Aktionen werden sicher auf dem UI-Thread ausgeführt; Quellen bleiben
  schreibgeschützt und Entwürfe durch den vorhandenen Schutzdialog gesichert.
- Aktivierung und Abmeldung berücksichtigen die Lebensdauer des Fensters.
- Native Plattformadapter und tatsächliche Abfragen prüfen; ein internes Baum-Dump
  allein belegt keine vollständige Screenreader-Nutzung.
- Systemvorgaben für reduzierte Bewegung und Kontrast sowie Schrift-Fallback
  zusätzlich anbinden und prüfen.

Die [native Ordnerwahl von SDL3](https://wiki.libsdl.org/SDL3/SDL_ShowOpenFolderDialog)
ist asynchron. Ihre Ergebnisse müssen auf den UI-Thread gelangen; Abbrechen,
Fehler und ein inzwischen geschlossener Dialog dürfen keine falsche Navigation
oder Datenänderung auslösen. Tests verwenden eigene temporäre Daten und verändern
keine persönlichen Einstellungen des laufenden Systems.

## Implementierter Umfang in 0.5.0

Fenster, modale Aufgaben, Schaltflächen, Suche, Eingabefelder, Projektsterne,
Lesedokument und Editor erhalten Rollen, Namen, Werte und Fokus. Sterne bilden
zusätzlich eine native Dokumentliste; sichtbare Sterne tragen ihre tatsächlichen
Punktpositionen. Editorwerte sind UTF-8-Markdown, noch keine semantisch aufbereitete
Überschriften-/Absatzstruktur. Das ist eine verbleibende Grenze.

Native Aktionen landen in einer begrenzten Warteschlange und werden auf dem
UI-Thread verarbeitet. Dokument-/Projektwechsel und neu aufgebaute Bedienelemente
invalidieren alte Aktionen. Native Knotenkennungen werden nach Kontextwechseln
nicht wiederverwendet. Quellen bleiben schreibgeschützt. Der vorhandene Entwurf-
und Speicherkonfliktschutz gilt auch für native Aktionen.

Der lokale macOS-Test fragt den wirklichen NSAccessibility-Provider ab und benutzt
Schaltflächen, Texteingaben, Notizerstellung, Textauswahl und Speichern. Er prüft außerdem einen
Dokumentwechsel zwischen Callback und Ausführung sowie den lesbaren Dokumentwert.
Das ist eine Providerprüfung über die native Schnittstelle, keine von einem
Menschen mit VoiceOver durchgeführte Bedienabnahme.

Auf allen Systemen prüft derselbe Test zusätzlich Snapshot-/Aktionsvertrag,
Unicode-Zeichenpositionen, ungültige Werte, alte Knoten, Schreibschutz und die
Queue-Grenze. Bei der ersten Adapterabnahme fehlten noch Windows-UIA-/Linux-AT-SPI-Clientabfragen;
die unten dokumentierte Ergänzung hat diese für den genannten Ablauf bestanden.
Weitere offene Punkte: vollständige passive Dialog-/Hilfetexte, graphemgenaue
Textläufe und Zeilengeometrie, zurückhaltende Fortschrittsansagen, reale assistive
Bedienung und große Dokumente mit laufender Animation. OS-Kontrast, reduzierte
Bewegung und Schrift-Fallback sind eigenständige offene Arbeiten.

Die native macOS-Auswahlersetzung über setAccessibilitySelectedText: ist im
verwendeten Adapter nicht verfügbar. Die Auswahl und das Setzen des Gesamtwerts
sind getrennt geprüft; eine vollständig assistive Editorbedienung ist noch offen.


## Native Clientprüfung ergänzen

Der nächste Prüfschritt ergänzt C-Clients für Windows UI Automation und Linux
AT-SPI. Das ist zunächst eine implementierte Prüfung, noch kein bestandener
Plattformnachweis. UIA benutzt einen eigenen COM-MTA-Thread; die App verarbeitet
weiter ihre Fensterereignisse. Quelle:
[Microsoft: UIA-Threading](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-threading),
eingesehen 6. Oktober 2026. Der C-Client verwendet die offiziellen SDK-COM-Interfaces.

Unter Linux verwendet ausschließlich das Testziel libatspi. Diese zusätzliche
UI-Prüfabhängigkeit wird nicht in das Produkt eingebunden oder ausgeliefert.
CI fordert sie ausdrücklich an. Eine private D-Bus-Sitzung mit in-memory
GSettings aktiviert den Zugänglichkeitsbus. Der Client sucht ausschließlich die
eigene Prozesskennung und fragt Rollen, Namen, Textwerte und Aktionen ab.
[GNOME EditableText](https://gnome.pages.gitlab.gnome.org/at-spi2-core/libatspi/method.EditableText.set_text_contents.html),
[get_desktop](https://gnome.pages.gitlab.gnome.org/at-spi2-core/libatspi/func.get_desktop.html),
[init](https://gnome.pages.gitlab.gnome.org/at-spi2-core/libatspi/func.init.html) und
[exit](https://gnome.pages.gitlab.gnome.org/at-spi2-core/libatspi/func.exit.html),
eingesehen 6. Oktober 2026. Initialisierung und einmalige Freigabe beachten den
Bibliotheksvertrag; ein fehlender Client wird ausdrücklich als fehlende Prüfung
benannt und im vorgeschriebenen CI-Build nicht still übersprungen.

Geprüfter lokaler Stand dieses Prüfschritts: macOS-Providerregression besteht.
Die tatsächlichen Windows-/Linux-Ergebnisse werden nach dem nativen Lauf ergänzt.


Der [erste Clientlauf zu 9335404](https://github.com/Lulus792/SecondBrain/actions/runs/37514295061)
besteht auf Windows und macOS jeweils in Debug/Release einschließlich Paketen.
Windows fragt tatsächlich UIA-Namen/Rollen/Werte ab und verwendet Invoke/SetValue.
Linux scheiterte zunächst beim Linken des Testclients; GObject wird jetzt explizit
angefordert, weil der Testclient seine Referenzfunktionen direkt verwendet.
Der korrigierte Linux-Client baut und wird im folgenden Lauf nativ geprüft.
Die Builddiagnostik veröffentlicht getrennte begrenzte Compiler-Ausgabeströme;
die anfangs abgeschnittene GitHub-Annotation wird nicht als Compilerursache gewertet.


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


## 0.5.1: Dialogtitel und Erklärungstexte

Die sichtbaren statischen Dialog-/Hilfetexte werden beim Zeichnen in denselben
nativen Baum übernommen: Einführung, leere Listen, Sicherungshinweise, Vorschau,
Fehler, Tastaturhilfe und Entwurfswarnung. Eine modale Aufgabe entfernt Hintergrund-
texte und -aktionen. Ihr eigener Titel benennt den nativen Dialog. Texte und
Bedienelemente folgen innerhalb ihrer UI-Gruppe der Zeichnungsreihenfolge;
passive Texte erhalten keine Fokus-/Klick-Aktionen. Beschriftungen werden vollständig
kopiert und nicht mehr an der Projektname-Grenze abgeschnitten. Eingabefeldnamen
werden direkt am Feld geführt, ohne doppelte statische Beschriftung.

Die Kontextkennung gehört zum tatsächlich gezeichneten Zustand. Ein während des
Zeichnens gewechseltes Formular kann alte Aktionen nicht als neue Bedienelemente
übernehmen. Author-IDs ergänzen die stabilen nativen Kennungen zur Identifizierung.

Grundlage: [Apple Accessibility](https://developer.apple.com/design/human-interface-guidelines/accessibility),
am 6. Oktober 2026 erneut über den offiziellen DocC-Inhalt gelesen: Oberfläche und
Inhalt für VoiceOver beschreiben. Die konkrete C-Snapshot-Anbindung ist eigene
Umsetzung. Dialogtitel werden als Heading/Level 1 modelliert. Unter lokalem
macOS 14.6.1 liefert der Adapter die native Rolle `Heading`; die erwartete wörtliche
Zeichenfolge `AXHeading` war eine falsche Testannahme. Falls die Systemkonstante
NSAccessibilityHeadingRole vorhanden ist, vergleicht die Prüfung mit ihr.
Der ältere lokale AppKit-Stand besitzt diese Konstante nicht; sein Adapterwert
belegt allein keine VoiceOver-Überschriftennavigation.

Die lokale Providerprüfung umfasst vollständige Hilfezeile, Dialogtitel/Identifier,
Modalabschirmung, Warnung zu ungespeicherten Änderungen und Rückkehr. Zusätzlich
prüft der gemeinsame Vertrag 700 Zeichen lange Beschriftungen und abgelehnte
Aktionen auf reinem Text. Gesamt-/Plattformnachweise folgen nach Prüfung.
Markdown-Dokumentstruktur, Grapheme/Zeilen, Fortschrittsansagen und menschliche
assistive Navigation bleiben weitere Arbeiten.


Die erste native Plattformnachprüfung findet eine tatsächliche macOS-Abweichung:
Auf neuerem AppKit ist NSAccessibilityHeadingRole vorhanden, der unveränderte
Adapter liefert dort trotzdem Heading. Die eigene C-Anbindung passt ausschließlich
diese native Rolle an die Systemkonstante an. Statische Textknoten erhalten neben
ihrem Namen einen tatsächlichen lesbaren Textwert und Textlauf. Die ursprünglichen
fehlgeschlagenen Plattformläufe bleiben als Nachweise erhalten; die korrigierten
nativen Prüfungen folgen. Die Runtime-Anpassung und ihr enger Versionsbezug stehen
im [Abhängigkeitsverzeichnis](../third_party/README.md).


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
