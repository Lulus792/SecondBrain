# Bestandsschutz beim Paketwechsel

Stand:11.Oktober2026. SecondBrain aktualisiert bestehende Projektordner nicht
automatisch. Der dokumentierte Weg ist weiterhin: App schließen, neues Paket
separat entpacken, vorhandenen Arbeitsordner wieder öffnen. Der folgende
Prüfweg macht diesen Vertrag wiederholbar; er installiert keine Programme.

[verify_package_update.py](../tools/verify_package_update.py) startet zwei
entpackte Pakete nacheinander mit isolierten Projektdateien und Einstellungen.
Python ist nur das Entwicklerprüfwerkzeug, keine App-Laufzeitabhängigkeit.
Die Wurzeln der alten und neuen Pakete müssen zum ausführenden System passen.

```sh
python3 tools/verify_package_update.py \
  --before-root PFAD_ZUM_ALTEN_PAKET \
  --after-root PFAD_ZUM_NEUEN_PAKET \
  --output build/update-check
```

Jeder Lauf bekommt einen neuen Unterordner; vorhandene Projekte und
Nutzereinstellungen werden nicht für die Prüfung verwendet. Die alte CLI
erstellt ein Projekt. Die Prüfdaten ergänzen eigene Metadaten mit unbekanntem
Zusatzfeld, eine UTF-8-Notiz mit CRLF/Graphemen, Eingang, Archiv und eine
Binärdatei. Ein separates Minimalmetadaten-Projekt muss weiter lesbar bleiben;
ein Projekt mit unbekanntem Schema2 muss weiterhin abgewiesen werden.

Die App wird zunächst mit der alten, dann mit der neuen Version und schließlich
erneut mit der alten Version gestartet. Die neue Version erhält **keine**
Arbeitsordner-/Projektargumente und muss den gespeicherten Ort sowie dieselbe
Notiz wiederherstellen. Nach jedem Abschnitt werden sämtliche Dateipfade
und SHA256-Werte verglichen. CLI-Kontext und Suche müssen zwischen den beiden
Versionen gleich bleiben; eine Neuerstellung mit bestehender Kennung muss
scheitern. Die neue Version erstellt und prüft eine Sicherung, stellt sie unter
freier Kennung wieder her und erhält alle Inhaltsbytes und Zusatzmetadaten.
Die drei App-Prozesse müssen echte Aufnahmen erzeugen. Der Report hält die
Programmidentitäten, Binärhashes, Dateihashes und Grenzen fest.

## Tatsächlicher lokaler Nachweis

Intel/macOS14.6.1: **0.9.63/92ab260e4203 →0.9.64/bef4816d3e40 →0.9.63**,
zwei unterschiedliche saubere Release-Pakete. Alle20 Arbeitsordnerdateien
bleiben bytegleich. Gespeicherte Notiz, Projekt und Darstellung bleiben erhalten.
Kontext-/Suchausgaben stimmen überein, Sicherung und Wiederherstellung bestehen.
Die alten, neuen und Rückkehr-Aufnahmen wurden angesehen: dieselbe eigene
Erkenntnis und ihr Originalpfad sind sichtbar. Die unterschiedenen Erstellmotive
und die korrigierte Sternlabelpriorität sind im neuen Paket sichtbar.

Originale:
`build/package-update-verification/Update ü 2ptc_9rz/report.json`,
`before.bmp`, `after.bmp`, `rollback.bmp`; zusätzlich betrachtete PNG-Kopien.
Die drei Bilder verwenden150Prozent Schrift und1000×700 logische Fenstergröße.
Der erste Entwicklungsversuch hatte den Wiederherstellungs-Arbeitsordner im
Prüfer nicht angelegt und wurde deshalb vom bestehenden CLI-Vertrag abgewiesen.
Nach Korrektur dieses Prüferfehlers besteht der vollständige Lauf. Es wurde
dafür keine App-Funktion verändert.

Die reguläre [Paketprüfung](../tests/test_package.cmake) verwendet ebenfalls
diesen Prüfer. Dort zeigen beide Wurzeln auf dasselbe entpackte Paket:
Das belegt Bestandsschutz beim Wiederöffnen, **keinen** Wechsel zwischen zwei
Versionen. `distinct_builds` im Report macht den Unterschied sichtbar.
Neue native Windows-/Linux-/ARM64-Paketnachweise werden gesondert festgehalten.
Die lokale erweiterte Paketprüfung des unveränderten0.9.64-Archivs besteht:
Wiederöffnungsprüfung mit20 Dateihashes,126 Desktop-,167 Tastatur- und75
Sicherungsbedingungen, zwei Einstellungsprozesse, Runtime-/Versionsprüfungen
und CLI-Sicherung. Original: `build/package-update-verification/package-check/`.

## Grenzen

Die Prüfung startet echte Snapshotprozesse, keine menschlichen Editor-/
Screenreader-Sitzungen. Sie ersetzt keine Signierung, Notarisierung, Installer-
oder Stromausfallprüfung. Der genaue historische Versionswechsel ist lokal
Intel/macOS geprüft; daraus folgt keine allgemeine Rückwärtskompatibilitätszusage
für künftige Datenformate. Die eigene [Migrationspolitik](DATENVERTRAG.md) und
die offenen [Release-Abnahmen](RELEASE.md) bleiben maßgeblich.

## Windows-Fixture nachgeprüft

CI38173351172 zu a5a5127 besteht in Linux-Debug/Release einschließlich Paket
sowie Windows-Debug. Windows-Release114570435578 besteht die Produktprüfung,
scheitert aber im neuen Paket-Prüfer: Die manuell geschriebene Einstellungsdatei
wurde im Python-Textmodus auf Windows mit CRLF erzeugt. Das native Format
verlangt LF. Die tatsächliche `before.bmp` zeigt deshalb korrekt „Einstellungsdatei
ist beschädigt oder hat eine unbekannte Version.“. Das ist ein Prüferfehler,
kein belegter Verlust gespeicherter Nutzerdateien.

Die Fixture schreibt jetzt explizite UTF-8-Bytes. Eine kontrollierte Windows-
Textmodus-Übersetzung im Python-Test bestätigt sowohl die vorherige CRLF-Ursache
als auch LF und erhaltene Unicode-/Prozentpfade des korrigierten Produzenten.
Der tatsächliche Mac-Wechsel63→64→63 besteht erneut mit20 erhaltenen Dateien.
Originale: `build/package-update-verification/windows-release-failure.log`,
SHA256-geprüftes Windows-Artefakt11714972949 (`windows-evidence.zip`), dessen
betrachtete `windows-update-before.png`, sowie `lf-regression/`.
Die neue native Windows-Paketprüfung folgt nach dem Push; ihre Aussage bleibt
bis zum tatsächlich ausgeführten Ergebnis offen.
