# Einstellungen und letzter Arbeitsstand

Stand: 6. Oktober 2026. Implementiert ab Entwicklungsversion 0.3.1.

## Bedienung

Die App merkt sich beim regulären Beenden den Arbeitsordner, das Projekt und die
zuletzt geöffnete Notiz. Helle oder dunkle Darstellung, Schriftgröße, reduzierte
Transparenz, reduzierte Bewegung und Fenstergröße werden ebenfalls wiederhergestellt.
Ungespeicherte Dokumente bleiben durch den vorhandenen Schutzdialog gesichert;
die Einstellungsdatei enthält keinen Dokumententwurf.

„Arbeitsordner öffnen“ bietet neben dem Pfadfeld „Ordner auswählen“. Der native
Ordnerdialog liefert zunächst einen Vorschlag im Pfadfeld. Erst „Öffnen“ wechselt
den Arbeitsordner. Abbruch erhält den vorherigen Wert. Verspätete Antworten eines
bereits geschlossenen Dialogs werden verworfen. Tab erreicht beide Eingabewege.

Ist der letzte Arbeitsordner nicht erreichbar, meldet die App dies und ersetzt
seine gespeicherte Angabe nicht automatisch. Nach bewusster Auswahl eines gültigen
Arbeitsordners wird dieser beim Beenden gespeichert. Eine gelöschte letzte Notiz
verhindert das Öffnen ihres weiterhin vorhandenen Projekts nicht.

## Datei und Bestandsschutz

Die Konfiguration liegt im nutzerspezifischen App-Verzeichnis von SDL_GetPrefPath
mit Organisation `Lulus792`, Anwendung `SecondBrain` und Datei `settings.conf`.
Unter macOS ist dies gewöhnlich `~/Library/Application Support/Lulus792/SecondBrain/`,
unter Windows das entsprechende Roaming-AppData-Verzeichnis und unter Linux
`$XDG_DATA_HOME/Lulus792/SecondBrain/` beziehungsweise `~/.local/share/Lulus792/SecondBrain/`.
Die tatsächlich zurückgegebene OS-Adresse ist maßgeblich.

`--settings DATEI` verwendet eine ausdrückliche Konfigurationsdatei, etwa für
isolierte Tests. `--workspace ORDNER` hat beim Start Vorrang vor dem gespeicherten
Arbeitsordner. Die normalen Selbsttests und Screenshots ohne `--settings` lesen
und schreiben keine Nutzereinstellungen.

Das eigene C-Modul verwendet ein versioniertes UTF-8-Textformat mit prozentkodierten
Pfadwerten, neun Pflichtfeldern und höchstens 32 KiB. Unbekannte Versionen,
Doppelfelder, ungültige Werte und beschädigte Dateien werden gemeldet und nicht
überschrieben. Speichern erfolgt über eine exklusiv angelegte temporäre Datei und
atomischen Austausch. Eine zwischenzeitliche Änderung durch eine andere Instanz
wird vor dem Austausch geprüft und als Konflikt gemeldet. Dies ist keine
prozessübergreifende Transaktionssperre; ein gleichzeitiger Austausch nach der
letzten Prüfung bleibt eine bekannte Grenze des aktuellen Dateiverfahrens.

## Nachweise und Grenzen

Die Prüfungen verwenden private Testdaten. `settings-persistence` prüft Format,
UTF-8, Konflikte und Bestandsschutz; `preferences-restart` verbindet diese Werte
mit neu erzeugten SDL-Fenstern, Projektposition, Darstellung und verspäteten
Dialogantworten. `preferences-process-restart` startet die App zweimal als
getrennte Prozesse und vergleicht die isolierte Konfiguration.

Der native Ordnerdialog basiert auf SDL3. Sein Öffnen und seine tatsächliche
Bedienung mit Maus, Tastatur und Screenreader müssen zusätzlich auf allen drei
Zielsystemen interaktiv abgenommen werden. Simulierte Rückgabeereignisse sind
kein solcher Nachweis. Automatische Übernahme von Systemdarstellung und
Systembewegung sowie ein fertiger erster Start bleiben Release-Aufgaben.

## Originalquellen

- [Apple HIG: Settings](https://developer.apple.com/design/human-interface-guidelines/settings),
  öffentliche Dokumentationsdaten am 6. Oktober 2026 gelesen: sinnvolle Defaults,
  aufgabenbezogene Optionen im Kontext und Achtung globaler Systemeinstellungen.
- [Apple HIG: Modality](https://developer.apple.com/design/human-interface-guidelines/modality),
  am selben Tag erneut gelesen: begrenzte Aufgabe, sichtbarer Abbruch und Rückkehr.
- [SDL3: SDL_GetPrefPath](https://wiki.libsdl.org/SDL3/SDL_GetPrefPath),
  gelesen am 6. Oktober 2026: nutzer- und appspezifischer Pfad; Rückgabe freigeben.
- [SDL3: SDL_ShowOpenFolderDialog](https://wiki.libsdl.org/SDL3/SDL_ShowOpenFolderDialog),
  gelesen am 6. Oktober 2026: asynchrone Rückgabe, Abbruch/Fehler, möglicher fremder
  Callback-Thread und Linux-Portalabhängigkeit. Die App pumpt weiter SDL-Ereignisse.

## Asynchrone Fenstergrößen

SDL kann Größenänderungen asynchron ausführen. Wiederherstellung und Speicherung
warten daher mit SDL_SyncWindow auf den Abschluss, bevor sie die tatsächliche
Größe verwenden. Ein fehlgeschlagener Größenwechsel wird gemeldet und ersetzt
keine gespeicherte Konfiguration. Die gezielte Prüfung wartet ebenfalls auf den
Größenwechsel; eine bloße Anfrage ist kein Beleg für eine bereits geänderte Größe.
[SDL_SyncWindow](https://wiki.libsdl.org/SDL3/SDL_SyncWindow), Originaldokumentation
und SDL3-3.2.30-Quellcode am 6. Oktober 2026 gelesen.
