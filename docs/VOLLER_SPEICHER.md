# Sicherung auf begrenzten, tatsächlich vollen Testvolumes

Stand: 12. Oktober 2026. Entwicklerprüfung des vorhandenen C-Kerns; keine neue
Anwendungsversion oder Laufzeitabhängigkeit.

## Ablauf und Bestandsschutz

Der gemeinsame Prüfer `tests/volume_cases.py` verwendet das Produktions-CLI und
den vorhandenen C-Fortschrittsworker. Fehler werden nicht in den Dateizugriff
injiziert. Ein eigenes, vorher identifiziertes Testvolume wird bis zu einem
wirklichen Speicherplatzfehler gefüllt. Danach bleiben 128 KiB frei, damit die
Sicherung beziehungsweise Wiederherstellung zunächst Teilfortschritt macht und
dann am verbleibenden Platz scheitert. Das Quellprojekt enthält einen 1-MiB-Anhang.

Der Fortschrittscallback muss mindestens 64 KiB bestätigen. Der C-Kern muss
SB_IO zurückgeben; ein Teilarchiv oder Teilprojekt darf nicht veröffentlicht sein.
Originalprojekt, vorhandene Sicherung und bestehendes Zielprojekt werden vor/nach
bytegenau verglichen. Kontrollierte temporäre Daten müssen entfernt sein. Nach
Entfernen des Füllers gelingen vollständige Sicherung und Wiederherstellung.

Ein unabhängiges Schreiblimit beendet den Füllversuch auch ohne Speicherfehler;
die Schleife darf nicht unbegrenzt einen falschen Mountpunkt füllen. Vor dem
Versuch prüfen die Plattformadapter zusätzlich Identität und Kapazität.

## Plattformwege

- **macOS:** bisheriges eigenes 32-MiB-HFS+-Image, hdiutil-Mountantwort,
  tatsächlicher Mountpunkt, anderes Gerät und statvfs-Kapazität. Die Szenarien
  sind jetzt mit den anderen Plattformen geteilt; das Image wird getrennt.
- **Linux:** neues 32-MiB-ext4-Image, eigener Loop-Handle, Mountpunkt mit anderem
  Gerät und ext4-Typ. Die Loop-Backing-Datei muss genau das neu angelegte Image
  sein. Nur der eigene Mount und Loop-Handle werden getrennt. Root oder
  nichtinteraktives sudo wird für diesen ausdrücklich eingeschalteten Test benötigt.
- **Windows:** neues 64-MiB-VHD, eigene NTFS-Partition und zuvor freier
  Laufwerksbuchstabe. DiskPart wählt keine nummerierten physischen Datenträger aus
  und führt kein `clean` aus. Die Storage-Abfrage ordnet das Volume dem erzeugten
  VHD zu; Buchstabe, NTFS, Label und Kapazität müssen stimmen. Das eigene VHD wird
  getrennt. Dieser ausdrücklich eingeschaltete Entwicklerweg benötigt Administratorrechte.

Windows/Linux registrieren den zusätzlichen Test nur mit
`-DSB_TEST_REAL_VOLUMES=ON`. Die Kern-CI schaltet ihn in Debug und Release ein;
Standardbuilds verlangen keine zusätzlichen Mountrechte. Der bisherige Mac-Test
bleibt regulär registriert. JSON-Berichte werden auch bei erfolgreicher Kern-CI
als Artefakte erhalten. Ein Berichtsfeld `detached=true` wird erst nach erfolgreichem
Trennen geschrieben.

Technische Originale, am 12. Oktober gelesen:
[DiskPart-Skripte](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/diskpart-scripts-and-examples),
[Attach VDisk](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/attach-vdisk),
[Assign](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/assign)
und [ext4](https://www.kernel.org/doc/html/latest/filesystems/ext4/overview.html).
Microsoft beschreibt eine Pause zwischen aufeinanderfolgenden DiskPart-Aufrufen;
der Test berücksichtigt 15 Sekunden vor dem Trennen.

## Ausgeführt und noch offen

Der gemeinsame Ablauf besteht lokal auf dem echten HFS+-Volume. Beide Versuche
bestätigen 70.436 Bytes Teilfortschritt vor SB_IO; Originale bleiben unverändert,
beide Wiederholungen gelingen und das Image ist getrennt. Sieben lokale
Negativprüfungen verweigern fremde/zu große Mounts oder falsche Dateisysteme vor
Eintritt in den Füllablauf und prüfen Aufräumen bei Fehlern. Die externe Mount-
Metadatenantwort ist in diesen Negativprüfungen kontrolliert; sie sind keine
nativen Windows-/Linux-Dateisystemnachweise.

Alle 35 lokalen Release-Kernprüfungen bestehen (26,95 s), einschließlich echter
HFS+-Abnahme und Prozessabbruchprüfung. Originale:
`build/native-volume-verification/guards.log`, `macos.log`, `core.log` und
`macos/Speicher ü 0sbxw7pm/report.json`.

Die ersten tatsächlichen Windows-/Linux-Läufe stehen noch aus. CI-Konfiguration
allein schließt die Lücke nicht. Die Prüfung betrifft Sicherung/Wiederherstellung
auf diesen begrenzten virtuellen Dateisystemen; Speichern von Notizen/Einstellungen,
weitere Dateisysteme, Netzlaufwerke und physische Persistenz bleiben getrennte
Abnahmen. Ein virtuelles Volume und ein Prozesskill sind kein Stromausfall.
Die installierte App bleibt 0.9.71 / 9867139fb2ac; 1.0 bleibt ohne Nutzerfreigabe offen.


## Erste native Linux-Abnahme und Windows-Aufbaukorrektur

CI38188340574 zu 2380173 besteht auf Linux in Debug und Release mit jeweils
35 Kernprüfungen, einschließlich echtem ext4-Volume. Linux-Releasejob
114615455135 bestätigt backup-real-volume in 1,11 s. Der Windows-Aufbau scheitert
vor Volume-Erzeugung an der gewählten UTF-16-Skriptkodierung: DiskPart zeigt seine
Befehlsübersicht. Der Prüfer schreibt die Skripte jetzt ohne BOM als ASCII und
verlangt dafür einen ASCII-Backing-Pfad. Die Projekt-/Archiv-/Wiederherstellungspfade
im gemeinsamen C-Ablauf enthalten weiterhin Unicode; die Backing-Pfadgrenze
betrifft nur diesen Entwicklerprüfer. Sieben Guard-Prüfungen und der HFS+-Versuch
mit zusätzlichen Unicode-Pfaden bestehen lokal. Neue Windows-Abnahme folgt;
kein NTFS-Erfolg wird aus dem bisher fehlgeschlagenen Aufbau abgeleitet.


## Native ext4-/NTFS-Abnahme nach der Kodierungskorrektur

[CI38188559402](https://github.com/Lulus792/SecondBrain/actions/runs/38188559402)
zu 30f867ac0bac bestätigt alle vier Windows-/Linux-Kernjobs mit jeweils 35 Tests.
Windows Release 114616125745, Debug 114616125770; Linux Release 114616125595,
Debug 114616125740. Die neue Volume-Prüfung ist in allen vier Konfigurationen
enthalten. Mac-CI weiterhin queued; der lokale HFS+-Nachweis bleibt getrennt.

Die zwei Release-Artefakte wurden heruntergeladen und gegen die GitHub-Digests
geprüft. JSON-Berichte bestätigen für beide Vorgänge auf beiden Dateisystemen:
ENOSPC, 70.436 Bytes Teilfortschritt, unveränderte Originale, erfolgreichen erneuten
Versuch und `detached=true`. Unicode-Projekt-, Archiv- und Wiederherstellungspfade
sind Teil des gemeinsamen neuen Ablaufs.

| Nachweis | NTFS / Windows | ext4 / Linux |
| --- | --- | --- |
| Tatsächliche Dateisystemkapazität | 65.990.656 Bytes | 27.234.304 Bytes |
| Artefakt | 11720625762 | 11719984807 |
| ZIP-SHA256 | ce3a9496db359ec4bae43e3f62d61abcc0ca968e982f83f3a11b6b573bf9900e | 949ed860c66d377ee789e71cfd1ac7686843e877f93508c88e5091a68c049222 |

Originale: `build/native-volume-verification/native-summary.json`,
`windows-release-30f867a.zip/json`, `linux-release-30f867a.zip/json`.
Die frühere Formulierung „erste native Läufe stehen aus“ beschreibt den damaligen
Zwischenstand. Dieser Abschluss betrifft Sicherung/Wiederherstellung, nicht die
oben benannten weiteren Schreib-/Persistenzabnahmen. Kein 1.0-Abschluss.


## Notizen und Einstellungen: neuer Prüfstand

Der vorhandene C-Testworker ruft jetzt zusätzlich die unveränderten Produktions-
APIs `sb_note_save` und `sb_settings_save` auf. Bestehende und neue Notizen sowie
bestehende und neue Einstellungsdateien erhalten jeweils einen echten
Speicherplatzfehler. Alte Datei und Projektdateien müssen bytegleich bleiben;
neue Teilziele und temporäre Schreibdateien dürfen nicht zurückbleiben.
Der im C-Aufrufer geladene Entwurf wird zusätzlich vor/nach per Hash verglichen.
Nach Entfernen des Füllers gelingen dieselben Schreibaufrufe mit passenden Inhalten.

Große Füllschreibvorgänge können trotz ENOSPC noch Platz für kleine Dateien
übrig lassen. Der Füller arbeitet deshalb anschließend mit 4-KiB-, 512-Byte-
und 1-Byte-Schreibvorgängen bis zum Fehler. Erst danach wird der gewünschte
Restplatz freigegeben: 128 KiB für Notizen, kein zusätzlicher Platz für Einstellungen.
Ein gültiger langer Unicode-Pfadwert erzeugt eine Einstellungsdatei größer als
4 KiB. Die Datei muss damit Datenblöcke benötigen; ein kleiner Inhalt könnte
gegebenenfalls in Metadaten eines Dateisystems passen. Der Pfadwert muss zur
Dateiformatgrenze passen, benötigt für die Speicherformatprüfung keinen
existierenden Arbeitsordner. Normale kurze Konfigurationen werden dadurch
nicht als grundsätzlich unspeicherbar auf einem vollen Dateisystem dargestellt.

Lokal auf dem echten HFS+-Image bestehen alle vier neuen Fälle, beide bisherigen
Backupfälle, Bestandsschutz, Wiederholung und Trennen. 35 lokale Release-Kern-
prüfungen bestehen (13,82 s); nach Ergänzung des C-Entwurfsvergleichs bestehen
fünf abschließende betroffene Prüfungen (3,20 s). Originale:
`build/volume-write-verification/core-final.log`, `targeted-final.log`
und `macos-final-report.json`. Der erste Versuch zeigte den unzureichenden großen
Füllschreibvorgang und wurde gezielt korrigiert; kein Produktfehler beobachtet.
Neue native Windows-/Linux-Nachweise für diese vier Fälle stehen noch aus.
App-Code und installierte 0.9.71 bleiben unverändert; physische Persistenz und
andere Dateisysteme bleiben getrennte Abnahmen.


## Anwendungsmodell beim Speichern aus dem Schutzdialog

Ein zusätzlicher Workeraufruf benutzt das unveränderte `SBApp`-Modell: Notiz
laden, tatsächlichen Editorpuffer ändern, Beenden anfordern und „Änderungen
speichern“ entscheiden. Bei SB_IO müssen Editorinhalt, alte Revision,
Änderungszustand, Schutzdialog und ausstehender Beenden-Befehl erhalten bleiben;
`quit` darf nicht gesetzt sein. Bei erfolgreichem Aufruf mit freiem Platz muss
der Entwurf gespeichert und der geschützte Wechsel abgeschlossen werden.
Der Vergleich nach Platzfreigabe startet einen neuen Testprozess; er behauptet
keine bereits gemessene Wiederholung in derselben laufenden UI-Sitzung.

Die neue fünfte Schreibprüfung besteht lokal auf HFS+, zusammen mit fünf
betroffenen CTest-Gruppen (4,10 s). Der Testworker verlinkt dafür das eigene
C-Anwendungsmodell; Produktionscode und Runtimebibliotheken sind unverändert.
Die UI-Codeanalyse zeigt außerdem: `result` speichert die Fehlermeldung, öffnet
die Karte und verändert den Modellpuffer nicht; die Revision wird in
`sb_app_save` erst nach erfolgreicher Dateispeicherung übernommen. Das ist kein
interaktiver Desktop-/Screenreader-Nachweis. Original:
`build/volume-write-verification/model-targeted.log`.
