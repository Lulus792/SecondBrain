# Plattformprüfung des SecondBrain

Windows, macOS und Linux sind verbindliche Zielplattformen. Die erste Version
benötigt Python ab 3.10 und verwendet ausschließlich dessen Standardbibliothek.
Markdown und JSON werden in UTF-8 geschrieben; generierte Dateien verwenden LF.

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
der Symlink-Test wurde ausgeführt. Der GitHub-Workflow wird nach dem Push ausgewertet.
Die CI-Matrix enthält Windows, macOS und Ubuntu, jeweils mit Python 3.10 und 3.14.
Eine vorhandene Workflow-Datei ist allein kein Nachweis bestandener Prüfungen.

## Grenzen

Die Prüfungen belegen das getestete Verhalten des Generators. Sie bewerten
keine KI-Integration oder Drittprogramme wie Obsidian. Die lokale Physim-Instanz
enthält Quellenverweise und dokumentierte Aussagen, keine neue Physim-Abnahme.

Wissensbasen bleiben bei einem Betriebssystemwechsel lesbar. Lokale Projektpfade
müssen am Zielrechner erreichbar sein. Absolute Laufwerksverweise sind an den
jeweiligen Rechner gebunden; relative Verweise bleiben bei gemeinsamem
Verschieben des Projekts und seiner Wissensbasis nutzbar.
