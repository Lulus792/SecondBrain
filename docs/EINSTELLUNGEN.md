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
Pfadwerten und höchstens 32 KiB. Version 1 enthält neun Pflichtfelder,
Version 2 elf. Unbekannte Versionen,
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
kein solcher Nachweis. Systemvorgaben sind ab 0.6.0 im unten beschriebenen Umfang angebunden;
ein fertiger erster Start und interaktive Abnahmen bleiben Release-Aufgaben.

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


## Systemdarstellung und Zugänglichkeit ab 0.6.0

Lumen bleibt die Grundeinstellung. Systemdarstellung ist in Darstellung separat
wählbar. Manuell reduzierte Bewegung/Transparenz und erhöhter Kontrast bleiben
eigene gespeicherte Wünsche. Bekannte Systemvorgaben ergänzen sie: Bewegung wird
reduziert, Transparenz vermieden und Kontrast erhöht. Die App setzt diese Vorgaben
nicht zurück. Im Menü steht der Grund statt eines wirkungslosen Aktivierungsknopfs.
Erhöhter Kontrast vermeidet durchscheinende Flächen und stärkt Text, Ränder und Fokus.

Die native UI-Schicht liest NSWorkspace auf macOS, SystemParametersInfoW auf
Windows und das XDG-Settings-Portal auf Linux. Unter GNOME ergänzt ein verfügbares
GSettings-Schema enable-animations die Bewegungsvorgabe. Linux fragt mit begrenzter
Antwortzeit im Hintergrund; der UI-Thread wartet nicht auf das Portal. Fehlende
oder fehlerhafte Antworten ersetzen nicht den letzten gültigen Systemstand.
Eine gültige Rückkehr zu normalen Werten hebt den Systemanteil wieder auf; eigene
Wünsche bleiben erhalten. Portal-Schlüssel für Transparenz sind nicht standardisiert;
unter Linux bleibt die eigene Option dafür maßgeblich, Kontrast erzwingt Deckflächen.

Abfragen werden spätestens im nächsten Abfragezyklus (etwa eine Sekunde plus
Antwortzeit) übernommen. Helle/dunkle Darstellung wird nur bei bewusst gewählter
Systemdarstellung übernommen. Die App verändert keine OS-Einstellungen.

Formatversion 2 ergänzt follow-theme und contrast als Pflichtfelder. Version 1
wird vollständig gelesen; die zusätzlichen Werte bleiben zunächst aus. Erst
bewusstes Speichern schreibt Version 2. Beschädigte und unbekannte Versionen,
Doppelfelder und konkurrierende Änderungen bleiben geschützt. Automatische
Systemwerte werden nicht als eigene Auswahl in die Datei zurückgeschrieben.

Die gezielte lokale Prüfung besteht für Auflösung, Speichern, Altformat und
Neustart; native macOS-Flags werden read-only abgefragt. Linux testet ein privates
Settings-Portal mit tatsächlichen D-Bus-Nachrichten, Wechsel, Fehler und Erholung.
Der [Lauf zu 444471d](https://github.com/Lulus792/SecondBrain/actions/runs/37527601503)
besteht mit 18 Jobs, einschließlich 16 Desktoptests je Debug/Release und drei
entpackten Paketen. Lokal bestehen alle 16 Release- und ASan/UBSan-Tests. Die
vollständige Kontrastmessung, Windows-Custom-High-Contrast-Paletten und reale
Bedienung nach Änderungen der Systemsteuerung bleiben eigene Prüfungen.

Quellen, gelesen am 6. Oktober 2026:
[NSWorkspace Reduce Motion](https://developer.apple.com/documentation/appkit/nsworkspace/accessibilitydisplayshouldreducemotion?language=objc),
[SystemParametersInfoW](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-systemparametersinfow),
[XDG Settings](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.Settings.html),
[SDL_GetSystemTheme](https://wiki.libsdl.org/SDL3/SDL_GetSystemTheme),
[GNOME Schema](https://github.com/GNOME/gsettings-desktop-schemas/blob/master/schemas/org.gnome.desktop.interface.gschema.xml.in).
