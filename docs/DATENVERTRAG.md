# Datenvertrag der lokalen Entwicklungsversion

Stand: 6. Oktober 2026. Die Metadaten- und Textprüfung wird in 0.7.1 vereinheitlicht.
Dies legt das vorhandene lokale Format fest; ein vollständiger 1.0-Vertrag benötigt
zusätzlich die unten genannten Plattform- und Migrationsabnahmen.

## Projektordner und Metadaten

Ein Arbeitsordner enthält Projektordner mit portablen Kennungen: 1–64 ASCII-Zeichen,
Kleinbuchstaben, Zahlen und einzelne Bindestriche. Kein führender/abschließender
Bindestrich und keine reservierten Windows-Gerätenamen. Die Ordnerkennung ist
für die lokale Projektwahl maßgeblich. Ein manuell umbenannter portabler Ordner
bleibt lesbar, auch wenn seine historische Metadatenkennung noch anders lautet.

`brain.json` ist ein UTF-8-JSON-Objekt ohne BOM oder NUL-Zeichen. Neue Projekte
schreiben sechs Felder. Der Leser erhält bisher akzeptierte Minimalmetadaten:
`name` ist erforderlich, die übrigen bekannten Felder sind optional. Fehlende
Angaben werden beim Lesen nicht ergänzt und Dateien nicht automatisch migriert.

| Feld | Vertrag beim Lesen |
| --- | --- |
| `name` | Nicht leere UTF-8-Zeichenkette, höchstens 255 Bytes, keine ASCII-Steuerzeichen; reine Leerzeichen unzulässig |
| `schema_version` | Falls vorhanden: positive ganze Dezimalzahl, unterstützt wird 1; unbekannte Schemas werden abgewiesen |
| `template_version` | Falls vorhanden: positive ganze Dezimalzahl bis 4294967295; Herkunft der Vorlage, keine automatische Inhaltsmigration |
| `id` | Falls vorhanden: portable Kennung wie oben |
| `created` | Falls vorhanden: nicht leere Textangabe wie `name`; eigene Generatoren schreiben `YYYY-MM-DD`, der Leser erhält ältere Textangaben |
| `project_root` | Falls vorhanden: `null` oder UTF-8-Pfad unter 4096 Bytes ohne ASCII-Steuerzeichen; relatives oder absolutes Ziel darf derzeit fehlen |

Doppelte bekannte Felder sind unzulässig, auch wenn ein Schlüssel durch Unicode-
Escapes geschrieben ist. Unbekannte Zusatzfelder werden als JSON-Werte mitgelesen;
Objekt-/Arrayverschachtelung ist auf 32 Ebenen begrenzt. Sie werden beim Lesen
nicht umgeschrieben. Wie bisher werden doppelte unbekannte Schlüsselnamen nicht
fachlich ausgewertet. Kommentare, überflüssige Kommata, unvollständige Strings,
ungültige UTF-8-/Unicode-Sequenzen und Bytes hinter dem JSON-Objekt werden abgewiesen.
Die komplette Datei darf höchstens 16 MiB groß sein.

Grundlage der JSON-Grammatik ist [RFC 8259](https://www.rfc-editor.org/rfc/rfc8259),
gelesen am 6. Oktober 2026. Bekannte Feldtypen, Versionspolitik, Grenzen und
Ablehnung von NUL sind eigene Produktentscheidungen. Der Parser und die fachlichen
Prüfungen sind eigenes C; eine externe JSON-Bibliothek wird nicht verwendet.

Projektliste, Sicherungsprüfung und Wiederherstellung verwenden dieselbe Prüfung.
Ein nicht unterstütztes Schema erzeugt eine verständliche Meldung und wird nicht
als aktuelles Format verarbeitet. Ein Fehler beim Auflisten weist derzeit den
gesamten betroffenen Arbeitsordner zurück; einzelne beschädigte Projekte werden
noch nicht separat in einer verwendbaren Projektliste dargestellt. Dateien bleiben
unangetastet. Dieser Komfort-/Fehlerzustand ist vor 1.0 noch zu verbessern.

## Dokumente und Quellen

Notizen sind UTF-8-Text ohne NUL-Zeichen mit höchstens 16 MiB. Eingebettetes NUL
wird vor dem Kopieren in den Editor erkannt, damit kein unsichtbarer Rest beim
Speichern verloren geht. Die Dokumentliste prüft dieselbe Bedingung. Eine defekte
Datei wird gemeldet, nicht gekürzt, bereinigt oder überschrieben. Externe Quellen
verwenden denselben Textvertrag und bleiben schreibgeschützt. Bei abgewiesener
Quelle bleiben die bisherige Quelle und ein offener Entwurf erhalten.

Markdown ist das Speicherformat; der aktuelle Renderer ist kein vollständiger
CommonMark-Parser. Er stellt Absätze, einfache Überschriften, Aufzählungen,
fenced Codeblöcke und einfache Inline-Links dar. Unverstandene Syntax bleibt
Text. Schreiben und Sicherung erhalten den eigenen gespeicherten Markdown-Text.
Schriftabdeckung, Unicode-Textgeometrie und eine abschließende Markdown-Abnahme
bleiben Release-Aufgaben.

Anwendungspfade verwenden UTF-8 und Puffer unter 4096 Bytes; reale Dateisystem-
und OS-Grenzen können niedriger sein. Wissensunterordner haben eine Rekursionsgrenze
von 32 Ebenen. Die Sternkarte unterstützt 4096 Dokumente und 65536 Verweise; die
Dokumentliste ist davon unabhängig. Große Datenmengen benötigen noch Leistungsabnahme.

Binäre Anhänge können im Projekt liegen und gesichert werden, gehören aber nicht
in den Texteditor. Der [Sicherungsvertrag](SICHERUNG.md) begrenzt Archive auf
4096 Einträge und 256 MiB Inhalt und beschreibt Bestandsschutz sowie Abbruch.
Die [Einstellungsdatei](EINSTELLUNGEN.md) hat einen eigenen Versionsvertrag.

## Migration und Update

Ein App-Update verändert bestehende Projektinstanzen und Vorlageninhalte nicht
automatisch. Neue Vorlagen wirken nur auf neue Projekte. Es gibt derzeit keine
allgemeine Projektmigration. Eine spätere Schemaänderung benötigt einen eigenen,
versionierten Weg mit Sicherung, Vorschau, Bestandsschutz und Abbruchprüfung.
Ein größerer `template_version`-Wert allein rechtfertigt kein Überschreiben.

Wiederherstellen verwendet einen freien neuen Ordner und ersetzt ausschließlich
das JSON-Token der Kennung, beziehungsweise ergänzt es bei alten Minimalmetadaten.
Andere Felder und deren gespeicherte Darstellung bleiben erhalten. Das Original-
archiv und vorhandene Projekte bleiben unverändert.

## Nachweis und offene Abnahmen

`metadata-contract` prüft bekannte Feldtypen/Versionen, Minimalformat, unbekannte
Zusatzfelder, nicht terminierte Eingabepuffer, NUL, Neuidentifizierung und erhaltene
Originalbytes. Die Modellprüfung erhält Quelle und Entwurf bei abgewiesenem Text.
Gesamt-/Sanitizer- und Plattformresultate werden im [Umsetzungsstand](STATUS.md)
gesondert dokumentiert; Testquelltext allein ist kein Abnahmeergebnis.

Unterstützte OS-Mindestversionen, beschädigte Einzelprojekte in der UI, vollständige
Markdown-/Unicode-Abnahme, mögliche zukünftige Migrationen und tatsächliche
Pfad-/Volumegrenzen sind vor 1.0 noch separat zu bestimmen und zu prüfen.


Abschließender lokaler Nachweis: Der vollständige Desktoplauf prüfte alle 18
Tests; die neue Quellenregression hatte zunächst einen falschen relativen Link
und wurde korrigiert. Die abschließenden vier Release-Nachprüfungen bestehen
(0,49 Sekunden), einschließlich 122 Metadaten- und 89 Modellaussagen. Alle acht
reinen Kerntests bestehen (3,60 Sekunden); die vier gezielten ASan/UBSan-Wege
bestehen (2,68 Sekunden). Mit dem bisherigen Kern schlägt die neue Regression
beim unbekannten Schema 2 fehl (Exit 1). Plattform- und Paketabnahme zu 0.7.1
folgen gesondert. Es fand keine automatische Migration bestehender Daten statt.
