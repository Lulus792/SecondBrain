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

## Grenzen der Nachweise

Die Prüfungen belegen die jeweils benannten Abläufe. Der KI-Kontext wird erzeugt
und kopiert; ein externer KI-Anbieter wurde dabei nicht integriert. Die Physim-Instanz
enthält Quellenverweise und dokumentierte Aussagen, keine neue Physim-Abnahme.

Wissensbasen bleiben bei einem Betriebssystemwechsel lesbar. Lokale Projektpfade
müssen am Zielrechner erreichbar sein. Absolute Laufwerksverweise sind an den
jeweiligen Rechner gebunden; relative Verweise bleiben bei gemeinsamem
Verschieben des Projekts und seiner Wissensbasis nutzbar.
