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
Queue-Grenze. Tatsächliche Windows-UIA- und Linux-AT-SPI-Clientabfragen fehlen noch.
Weitere offene Punkte: vollständige passive Dialog-/Hilfetexte, graphemgenaue
Textläufe und Zeilengeometrie, zurückhaltende Fortschrittsansagen, reale assistive
Bedienung und große Dokumente mit laufender Animation. OS-Kontrast, reduzierte
Bewegung und Schrift-Fallback sind eigenständige offene Arbeiten.

Die native macOS-Auswahlersetzung über setAccessibilitySelectedText: ist im
verwendeten Adapter nicht verfügbar. Die Auswahl und das Setzen des Gesamtwerts
sind getrennt geprüft; eine vollständig assistive Editorbedienung ist noch offen.
