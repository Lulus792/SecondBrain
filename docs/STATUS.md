# Umsetzungsstand von SecondBrain

Stand: 6. Oktober 2026. Die Umsetzung ist aktiv; die vollständige Desktop-Anwendung
ist noch nicht fertiggestellt.

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
an KI-Werkzeuge. Es ersetzt die ausstehende eigene Oberfläche nicht.

## Ausgeführte Prüfungen

Auf dem lokalen Intel-Mac mit macOS 14.6.1 und AppleClang 16 bestehen Build und
Kernablauf. Ein zusätzlicher Lauf mit AddressSanitizer und UndefinedBehaviorSanitizer
besteht ebenfalls. Der native Kern liest das vorhandene Physim-Beispiel und
liefert Treffer aus Projektauftrag, Entscheidungen und Wissensnotiz.

Die Tests prüfen echte Dateien: Erstellung, Wiederöffnung, UTF-8, Suchtreffer,
Speicherkonflikte, Archivierung, Kontext und gültige beziehungsweise fehlerhafte
JSON-Metadaten. Die C17-CI-Matrix ergänzt die bisherigen Python-Prüfungen um
Windows, macOS und Linux in Debug und Release.
Der [C17-Lauf zu 4c99bb3](https://github.com/Lulus792/SecondBrain/actions/runs/37435200479)
besteht auf allen drei Systemen in beiden Buildprofilen.

## Implementierte UI Grundlage

SDL3 und Nuklear sind ausschließlich im UI-Ziel eingebunden. Die Grundlage
verarbeitet Texteingaben, stellt Schriften mit passender Pixeldichte dar und
bietet helle und dunkle Farben sowie vergrößerte Schrift. UI-Quellen, Versionen
und Lizenzen sind unter third_party dokumentiert.

Lokal bestehen Renderer- und Editorprüfungen, auch mit AddressSanitizer und
UndefinedBehaviorSanitizer. Sie prüfen Unicode-Paste, Auswahlersetzung,
Rückgängig/Wiederholen, lange Cursorpositionen, Kapazitätsfehler und mehrteilige
Texteingaben. Die tatsächliche Testdarstellung wurde betrachtet. Die UI-CI
prüft diese Grundlage zusätzlich mit nativen Fenstersystemen der drei Plattformen.
Ergebnisse werden nach dem jeweiligen Lauf ergänzt.

## Weiter ausstehend

Die eigene Oberfläche mit allen geplanten Arbeitsabläufen, Darstellung und
Bedienprüfung, Paketierung ohne Python-Laufzeit sowie die vollständigen Nachweise
auf den drei Zielplattformen stehen aus. Das aktive Ziel bleibt die vollständige
Umsetzung der Anwendung.
