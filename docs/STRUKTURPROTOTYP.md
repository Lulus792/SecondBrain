# Strukturprototyp

Die folgenden Befehle betreffen den vorhandenen Python-Prototyp. Für ihn genügt
Python 3.10 oder neuer; er hat keine zusätzlichen Paketabhängigkeiten. Das Datum
stammt aus der lokal eingestellten Betriebssystem-Zeitzone. Die C-Anwendung
ermöglicht diese Arbeit durch ihre eigene Oberfläche.

Im Ordner dieses Repositories unter macOS oder Linux:

```sh
python3 secondbrain.py new mein-projekt --name "Mein Projekt"
```

Unter Windows in PowerShell:

```powershell
py -3 secondbrain.py new mein-projekt --name "Mein Projekt"
```

Falls Python dort über `python` verfügbar ist, kann dieser Befehl entsprechend
verwendet werden. Alle Optionen sind auf den drei Plattformen gleich.

Das erstellt `brains/mein-projekt/`. Der Bezeichner darf Kleinbuchstaben, Zahlen
und einzelne Bindestriche enthalten, höchstens 64 Zeichen. Unter Windows
reservierte Gerätenamen werden auf allen Plattformen abgelehnt; der Anzeigename
ist frei wählbar.

Ein bestehendes Projekt verknüpfen:

```sh
python3 secondbrain.py new anderes-projekt --name "Anderes Projekt" --repo ../anderes-projekt
```

Ein eigenes Zielverzeichnis wählen:

```sh
python3 secondbrain.py new mein-projekt --name "Mein Projekt" --output /pfad/zum/projekt/second-brain
```

Der Generator verweigert jedes bereits vorhandene Zielverzeichnis. `--repo`
speichert einen Verweis auf das Projekt; eine automatische Übernahme oder
Auswertung von dessen Dateien erfolgt nicht. Relative Verweise bleiben bei
gemeinsamem Verschieben erhalten; bei getrenntem Verschieben musst du sie anpassen.

## Mit einem Projekt anfangen

1. `START.md` öffnen und in `PROJECT.md` Ziel und Erfolgskriterien ergänzen.
2. In `SOURCES.md` die verbindlichen Originaldokumente eintragen.
3. In `STATE.md` den aktuellen Stand und einen konkreten nächsten Schritt festhalten.
4. Ideen zunächst in `inbox/INBOX.md` sammeln.
5. Nach einem Arbeitsabschnitt relevante Ergebnisse und Entscheidungen einpflegen.

Für einen neuen KI-Chat kannst du diesen Auftrag verwenden und den Pfad ersetzen:

> Lies /absoluter/pfad/zum/second-brain/START.md und die darin genannten
> Kerndateien. Öffne anschließend die Quellen, die für meine Aufgabe relevant
> sind. Sage, was belegt, offen oder veraltet ist, und arbeite dann an meinem Auftrag.

Die Dateien werden dadurch zum externen Projektgedächtnis. Ihre Existenz allein
stellt weder ein automatisches Einlesen noch eine laufende Aktualisierung sicher.
Die Pflege erfolgt in dieser Version durch dich oder durch einen damit beauftragten
KI-Chat. Die jeweiligen Projektanweisungen bleiben maßgeblich.


## Prüfen

```sh
python3 -m unittest discover -s tests -v
```

Unter Windows: `py -3 -m unittest discover -s tests -v`.
