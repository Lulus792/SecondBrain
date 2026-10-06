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

## Grenzen

Die Prüfungen belegen das getestete Verhalten des Generators. Sie bewerten
keine KI-Integration oder Drittprogramme wie Obsidian. Die lokale Physim-Instanz
enthält Quellenverweise und dokumentierte Aussagen, keine neue Physim-Abnahme.

Wissensbasen bleiben bei einem Betriebssystemwechsel lesbar. Lokale Projektpfade
müssen am Zielrechner erreichbar sein. Absolute Laufwerksverweise sind an den
jeweiligen Rechner gebunden; relative Verweise bleiben bei gemeinsamem
Verschieben des Projekts und seiner Wissensbasis nutzbar.
