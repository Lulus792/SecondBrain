# Sicherung und Wiederherstellung

Stand: 6. Oktober 2026. Der eigene C-Kern, das Entwicklungswerkzeug und
die Desktop-Anbindung sind in Entwicklungsversion 0.4.0 implementiert.
Prüfumfang und verbleibende Abnahmen stehen unten.

## Inhalt und Bestandsschutz

Eine `.sbbackup`-Datei enthält alle regulären Dateien und Unterordner eines
Projektgedächtnisses einschließlich leerer Ordner und binärer Anhänge. Dateien
verknüpfter externer Repositories werden nicht eingesammelt. Verknüpfungen,
Gerätedateien und unlesbare Einträge führen zu einer Meldung; es wird nichts
stillschweigend ausgelassen. Gesichert wird der gespeicherte Dateistand.

Bestehende Sicherungsdateien werden nicht ersetzt. Das Ziel muss außerhalb des
zu sichernden Projektordners liegen. Der Kern erfasst Dateien, schreibt eine
exklusiv angelegte temporäre Datei, vergleicht das Projekt erneut und prüft die
fertige Sicherung vor ihrer Veröffentlichung. Erkannte Änderungen brechen ab.
Das ist keine prozessübergreifende Dateisystemsperre oder atomare Momentaufnahme
aller Quelldateien: Änderungen nach der letzten Prüfung bleiben möglich.

Wiederherstellen prüft zunächst die vollständige Sicherung, schreibt dann in
einen neuen temporären Ordner im Zielarbeitsordner und prüft beim zweiten Lesen
erneut. Erst das vollständige Ergebnis wird exklusiv unter dem gewählten freien
Projektordnernamen veröffentlicht. Bestehende Ordner, auch leere, werden niemals
ersetzt. Das `id`-Feld der Metadaten wird auf den gewählten Ordnernamen angepasst;
Notizen, Anhänge und unbekannte andere Metadatenfelder bleiben erhalten.

Abbruch und Fehler entfernen ausschließlich die in diesem Vorgang selbst
angelegten temporären Daten. Scheitert deren Entfernung, wird der verbleibende
Pfad gemeldet. Ein harter Prozessabbruch kann temporäre `.sb-restore-*`-Ordner
oder `.sb-backup-*`-Dateien zurücklassen; sie gelten nicht als fertige Projekte
oder Sicherungen. Es gibt noch keine automatische Bereinigung nach einem Absturz.

## Format und Grenzen

Das eigene Binärformat Version 1 verwendet explizite Little-Endian-Felder,
UTF-8-Pfade, SHA-256 pro Datei und eine Gesamtprüfsumme. Einträge sind eindeutig
geordnet und enthalten Typ, Pfad, Länge und Prüfsumme. Unbekannte Versionen,
unvollständige oder zusätzliche Daten, Prüfsummenfehler, absolute Pfade,
Traversal, doppelte Pfade und fehlende Elternordner werden abgewiesen.
Prüfsummen erkennen beschädigte Daten; sie signieren weder Urheber noch Herkunft.

| Grenze | Wert |
| --- | --- |
| Dateien und Unterordner | höchstens 4096 Einträge |
| Größe einer Datei | höchstens 16 MiB |
| Gesamte Dateiinhalte | höchstens 256 MiB |
| Tiefe | höchstens 32 Komponenten |
| Einzelner Pfadbestandteil | höchstens 255 UTF-8-Bytes |
| Relativer Pfad | weniger als 4096 UTF-8-Bytes |

Nicht portable Namen, Windows-Gerätenamen, Steuerzeichen, Backslash und
Windows-Sonderzeichen sowie Punkt/Leerzeichen am Ende einer Komponente werden
abgewiesen. ASCII-Groß-/Kleinschreibung darf keinen zweiten gleichnamigen Eintrag
erzeugen. Weitere Unicode-Normalisierungs- oder Großschreibkollisionen werden
durch exklusives Anlegen auf dem Zielsystem abgefangen; sie werden nicht
stillschweigend zusammengeführt. Rechte, Eigentümer, Zeitstempel und Extended
Attributes gehören nicht zu diesem Inhaltsformat. Zusätzliche Pfad- und
Namensgrenzen des Zielbetriebssystems und Dateisystems bleiben maßgeblich.

Das Veröffentlichen benötigt einen exklusiven Rename innerhalb desselben
Dateisystems. Windows verwendet MoveFileEx ohne Ersetzen, macOS RENAME_EXCL,
Linux renameat2 mit RENAME_NOREPLACE. Fehlende Dateisystemunterstützung führt zu
einem Fehler und keinem unsicheren Fallback. Daten werden vor Veröffentlichung
synchronisiert; ein Stromausfall auf beliebiger Hardware ist damit nicht abgenommen.

## Entwicklungswerkzeug

```sh
secondbrain-cli backup ARBEITSORDNER KENNUNG SICHERUNGSDATEI
secondbrain-cli inspect SICHERUNGSDATEI
secondbrain-cli restore SICHERUNGSDATEI ARBEITSORDNER NEUE-KENNUNG
```

Der Zielarbeitsordner muss vorhanden sein. Die normale Nutzung wird über die
Desktop-App erfolgen; diese Befehle dienen auch deren unabhängiger Prüfung.

## Desktop-Ablauf

Die erneut gelesenen Apple-HIG-Kapitel Progress indicators und Alerts begründen
sichtbaren, zutreffenden Fortschritt, eine sichere Abbruchmöglichkeit und knappe
Rückmeldungen im Aufgabenkontext. Eigene Übertragung: Sicherung/Wiederherstellung
als begrenzte Glaskarten-Aufgabe; tatsächliche Byte-/Eintragswerte je Phase,
keine erfundene Zeitprognose. Eine zunächst unbekannte Erfassung zeigt keinen
Prozentwert. Normale Erfolge erhalten eine lokale Meldung; Fehler bleiben mit
Wiederholen erreichbar. Bestehende Daten bleiben ohne zusätzlichen Warnungsdialog
vor Ersetzen geschützt, weil Ersetzen nicht angeboten wird.

Implementiert: native Dateiauswahl plus Pfadfeld, Vorschau des geprüften Sicherungsinhalts,
freier Zielordnername und Hintergrundarbeit mit Abbruch. Ein SDL-Tastaturdurchlauf
prüft diese Wege. Die tatsächlichen OS-Dateidialoge sind noch nicht interaktiv abgenommen.
Eine offene Bearbeitung wird vor Sichern bewusst gespeichert; ein Speicherkonflikt
verhindert den Beginn. Wiederherstellung lässt eine bestehende Bearbeitung erhalten.

Einstieg: Aktionen → Projekt sichern; Projekte oder Aktionen → Sicherung
wiederherstellen. Wiederherstellen verwendet den aktuellen Arbeitsordner.
Die geprüfte Vorschau enthält eine Gesamtprüfsumme; eine seitdem veränderte
Sicherungsdatei wird abgewiesen und muss erneut geprüft werden. Erfolgreiche
Wiederherstellung ergänzt die Projektliste und ersetzt keinen offenen Entwurf.
Gleichnamige Projekte werden dort über ihre Ordnerkennungen unterschieden.

Escape und Abbrechen räumen den laufenden Vorgang auf. Beenden während einer
Hintergrundarbeit wartet darauf und schützt anschließend weiterhin einen offenen
Entwurf. War eine Veröffentlichung vor dem Abbruch bereits abgeschlossen, wird
dies gemeldet. Fehler erscheinen oben im Formular; ihre vollständige Meldung
lässt sich mit sichtbarer Rückmeldung kopieren.

## Gezielte Nachweise

`backup-integrity` prüft vollständige Rundreise einschließlich binärer Anhänge,
leerer Ordner und Unicode, bekannte SHA-256-Testvektoren, Identität,
Pfad-/Prüfsummen-/Versionsfehler, Abbruch in den Phasen, Quellenkonflikte und
exklusives Veröffentlichen. `backup-write-failure` verwendet einen ausschließlich
im Testziel kompilierten Schreibfehler, um vollen Datenträger nach Teilfortschritt
und Fehler beim Identitätswechsel zu prüfen. Dies ist Fehler-Injektion, kein
physisch gefülltes Volume. Das Produktionsprogramm enthält diese API nicht.
`backup-cli-workflow` prüft das tatsächlich gebaute Produktionswerkzeug mit
Unicode-Pfaden und Bestandsschutz über getrennte Prozesse.

Lokal bestehen die sieben betroffenen Prüfungen in Release und mit ASan/UBSan.
Die Integritätsprüfung umfasst 482 Aussagen, die Schreibfehlerprüfung 27;
der Produktions-CLI besteht seine getrennte Prozessprüfung.

`backup-ui-workflow` verwendet dieselbe verpackte App und reine SDL-Tastaturereignisse:
Sichern einschließlich Entwurf, Vorschau, Wiederherstellen, erhaltene Bearbeitung,
belegte Namen, geänderte Vorschau, Save-Konflikt, Dialogantworten und Beenden.
Die Zusatzdatei im UI-Durchlauf hat 8 MiB. Gesehene Softwarebilder belegen ihre
genannten Fensterzustände; sie ersetzen keine interaktive native Dialogprüfung.

Weitere Abnahmen: native Dateidialoge, große reale Daten,
Speicherplatzbedingungen auf tatsächlichen Zielvolumes und Verhalten nach
hartem Prozessabbruch. Die vollständige Release-Liste bleibt maßgeblich.

## Originalquellen

Am 6. Oktober 2026 gelesen:

- [NIST FIPS 180-4](https://csrc.nist.gov/pubs/fips/180-4/upd1/final), SHA-256 als
  Grundlage; eigene C-Implementierung, keine Kryptografiebibliothek oder Zertifizierung.
- [Microsoft MoveFileExW](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexw),
  Verzeichnisse nur innerhalb eines Laufwerks; Ersetzen ist eine eigene Option.
- [Linux rename(2)](https://man7.org/linux/man-pages/man2/rename.2.html),
  RENAME_NOREPLACE und notwendige Dateisystemunterstützung.
- Offizielles lokal installiertes Apple-SDK, stdio.h: renamex_np/RENAME_EXCL;
  Kompilierung und tatsächlicher exklusiver Verzeichnistest auf Intel macOS.
- [Apple HIG Progress indicators](https://developer.apple.com/design/human-interface-guidelines/progress-indicators)
  und [Alerts](https://developer.apple.com/design/human-interface-guidelines/alerts),
  vollständige Best-practices-Passagen über Apples öffentliche Dokumentationsdaten
  gelesen, bevor die neue Desktop-Aufgabe entworfen wird.


UI-Prüfungen teilen die System-Zwischenablage. CTest serialisiert die betroffenen
Tests über einen Resource Lock innerhalb desselben Laufs. Getrennte CTest-Prozesse
auf demselben Desktop müssen ebenfalls nacheinander laufen. Ein versehentlicher
lokaler Überlappungsversuch erzeugte falsche eingefügte Pfade; diese Prüfung ist
kein Produktbefund und wird nach der serialisierten Nachprüfung getrennt vermerkt.


Der [0.4.0-Lauf zu d2d9c2b](https://github.com/Lulus792/SecondBrain/actions/runs/37508856383) besteht mit allen 18 Jobs.
14 Desktopprüfungen je Debug/Release und tatsächliche Paketrundreise bestehen auf
Windows x64, macOS ARM64 und Linux x64. Der endgültige Bedienlauf enthält 75,
Integrität 487 und injizierter Schreibfehler 27 Aussagen. Die lokalen Nachprüfungen
und verbleibenden Grenzen stehen in STATUS.md und PLATTFORMEN.md.

## Prozessabbruch und tatsächlich volles Volume, 7. Oktober 2026

`backup-process-kill` startet echte C-Kindprozesse mit dem unveränderten
Produktionskern und friert sie ausschließlich über dessen Fortschrittscallback
ein. Der Elternprozess beendet sie unter POSIX mit SIGKILL, unter Windows
mit TerminateProcess. Geprüfte Kontrollpunkte: Schreiben, Quellnachprüfung,
Sicherungsprüfung und unmittelbar vor Veröffentlichung; Wiederherstellung
nach Teilfortschritt und unmittelbar vor Veröffentlichung.

Nach jedem Abbruch bleiben Originalprojekt und vorhandene Sicherung bytegleich.
Es existiert kein veröffentlichtes Teilziel. Temporäre Reste bleiben erhalten;
ein erneuter vollständiger Versuch benutzt einen neuen temporären Pfad und
verändert die Reste nicht. Verborgene Wiederherstellungsreste erscheinen nicht
als Projekte. Eine vollständig geschriebene temporäre Sicherung unmittelbar
vor Veröffentlichung wird zusätzlich ausdrücklich geprüft und unter einem
neuen Namen wiederhergestellt. Das ist keine automatische Absturzbereinigung
oder Fortsetzung einer unvollständigen Datei.

`backup-real-volume` erzeugt auf macOS ein begrenztes, entbehrliches 32-MiB-
HFS+-Diskimage. Es prüft den tatsächlichen Mountpunkt und die Volumegröße,
bevor es das Volume bis zum echten ENOSPC füllt. Sicherung und Wiederherstellung
fehlschlagen nach bestätigtem Teilfortschritt; Teilziele werden nicht
veröffentlicht, temporäre Daten werden kontrolliert entfernt, vorhandene
Projekte bleiben bytegleich. Nach Freigabe des Platzes gelingen beide
Versuche. Das Image wird abschließend getrennt.

Lokal bestehen alle sechs Prozessabbruchfälle und beide HFS+-Speicherfehler.
Diese Testorchestrierung verwendet Python-Standardbibliothek und das Apple-
Werkzeug hdiutil; sie ist kein Bestandteil der normalen App. Windows-/Linux-
Prozessabnahme sowie weitere echte Dateisysteme werden erst nach ausgeführten
Prüfungen genannt. Ein beendeter Prozess ist kein Stromausfall: Persistenz
bei Hardware-, Kernel- oder Geräteausfall bleibt gesondert abzunehmen.
