# Sicherung und Wiederherstellung

Stand: 6. Oktober 2026. Der eigene C-Kern und das Entwicklungswerkzeug sind
implementiert. Die Anbindung an die normale Desktop-Bedienung folgt darauf;
sie ist mit diesem Dokument noch nicht als fertig oder abgenommen bezeichnet.

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
Attributes gehören nicht zu diesem Inhaltsformat.

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

## Desktop-Ablauf vor der Umsetzung

Die erneut gelesenen Apple-HIG-Kapitel Progress indicators und Alerts begründen
sichtbaren, zutreffenden Fortschritt, eine sichere Abbruchmöglichkeit und knappe
Rückmeldungen im Aufgabenkontext. Eigene Übertragung: Sicherung/Wiederherstellung
als begrenzte Glaskarten-Aufgabe; tatsächliche Byte-/Eintragswerte je Phase,
keine erfundene Zeitprognose. Eine zunächst unbekannte Erfassung zeigt keinen
Prozentwert. Normale Erfolge erhalten eine lokale Meldung; Fehler bleiben mit
Wiederholen erreichbar. Bestehende Daten bleiben ohne zusätzlichen Warnungsdialog
vor Ersetzen geschützt, weil Ersetzen nicht angeboten wird.

Geplant: native Dateiauswahl plus Pfadfeld, Vorschau des geprüften Sicherungsinhalts,
freier Zielordnername, Hintergrundarbeit mit Abbruch und vollständige Tastaturwege.
Eine offene Bearbeitung wird vor Sichern bewusst gespeichert; ein Speicherkonflikt
verhindert den Beginn. Wiederherstellung lässt eine bestehende Bearbeitung erhalten.

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

Weitere Abnahmen: echte UI-Abläufe, native Dateidialoge, große reale Daten,
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
