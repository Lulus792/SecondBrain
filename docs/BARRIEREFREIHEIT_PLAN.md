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
