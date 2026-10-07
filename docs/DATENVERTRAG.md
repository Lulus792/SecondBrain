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
als aktuelles Format verarbeitet. Ab 0.7.2 enthält die App-Projektwahl einzelne
nicht verfügbare Einträge mit Grund und Ordnerpfad, während gültige Projekte nutzbar
bleiben. Auch ein Ordner mit START.md und fehlendem brain.json erscheint als Fehler.
Gewöhnliche Ordner ohne Projektmerkmale werden weiter ignoriert. Fehler beim Lesen
des gesamten Arbeitsordners oder fehlender Speicher brechen die Erkennung ab.
Die strikte Kernfunktion sb_projects_list bleibt erhalten; sb_projects_scan liefert
zusätzlich Fehlerzustände je Eintrag. Keine der beiden verändert Dateien.

Beim Öffnen prüft das Modell die aktuellen Metadaten erneut. Ein Fehler beim Lesen
der Notizen wird dem betreffenden Projekt zugeordnet; der Start versucht danach
weitere Projekte. Sind alle nicht verfügbar, bleiben Einstieg, Neuanlegen,
Ordnerwechsel, Wiederherstellen und Prüfen erreichbar. „Liste erneut prüfen“ liest
neu ein und erhält offene Entwürfe. Nach einer Korrektur ist das Projekt wieder
wählbar. Die App repariert keine Metadaten automatisch. Die CLI listet gültige
Metadateneinträge auf stdout und Gründe auf stderr; Exit 1 kennzeichnet eine
unvollständige Liste. context/search/backup für andere Kennungen bleiben verfügbar.

## Dokumente und Quellen

Notizen sind UTF-8-Text ohne NUL-Zeichen mit höchstens 16 MiB. Eingebettetes NUL
wird vor dem Kopieren in den Editor erkannt, damit kein unsichtbarer Rest beim
Speichern verloren geht. Die Dokumentliste prüft dieselbe Bedingung. Eine defekte
Datei wird gemeldet, nicht gekürzt, bereinigt oder überschrieben. Externe Quellen
verwenden denselben Textvertrag und bleiben schreibgeschützt. Bei abgewiesener
Quelle bleiben die bisherige Quelle und ein offener Entwurf erhalten.

Markdown ist das Speicherformat; der aktuelle Renderer ist kein vollständiger
CommonMark-Parser. Ab 0.9.4 teilen Titel und Leseansicht eigene Regeln für
Absätze, ATX-/Setext-Überschriften, eingerückten und fenced Code. Inline-Links
sind begrenzt aufbereitet; Listen, Zitate und Tabellen haben weiterhin eine
vorläufige Textdarstellung. [Umfang und Grenzen](MARKDOWN.md). Schreiben und
Sicherung erhalten den eigenen gespeicherten Markdown-Text.
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


## Abnahme von 0.7.1

[Lauf zu b3c0af5](https://github.com/Lulus792/SecondBrain/actions/runs/37532681504)
besteht mit allen 18 Jobs: 18 Desktoptests je Debug/Release auf Windows x64,
macOS ARM64 und Linux x64 sowie drei entpackte Pakete. Lokal besteht das entpackte
Intel-Paket mit 126 Desktop-, 105 Tastatur- und 75 Sicherungs-UI-Aussagen sowie
Einstellungsprozessen und CLI-Sicherung. Archiv-SHA-256: `6bd2cea886e7e6f88335b8987cd1e25839f3b0b54ff969e84afe403114d24aba`.
Der weitere Release-Auftrag einschließlich Einzelprojekt-Fehlerzuständen bleibt aktiv.


Abschließender lokaler Nachweis: alle 20 Tests des vollständigen Release-Laufs
bestehen (190,75 Sekunden), der zusätzliche Produktions-CLI-Test besteht
(0,77 Sekunden). Alle zehn reinen Kerntests bestehen (3,72 Sekunden). Vier gezielte
ASan/UBSan-Wege bestehen (38,51 Sekunden): Projektfehler-UI, erster Start, native
Zugänglichkeit und Projektmodell. Die neuen Prüfungen enthalten 39 UI- und
46 Modell-/Erkennungsaussagen. Tatsächliche dunkle und helle Kontrastbilder mit
200 Prozent Schrift sind betrachtet. Native Plattform- und Paketabnahme zu 0.7.2
folgen gesondert; automatische Metadatenreparatur wurde nicht eingeführt.


## Prüfung laufender Schreibaktionen ab 0.7.3

Speichern, Notizkopien, Neuanlegen und Archivieren prüfen die aktuellen Metadaten
vor der Aktion. Unbekannte/beschädigte oder inzwischen entfernte Metadaten dürfen
auch über einen bereits geöffneten Projektzustand keine Schreibaktion ermöglichen.
Speichern und Verschieben prüfen zusätzlich die Metadatenrevision kurz vor dem
Veröffentlichen. Eine erkannte Änderung führt zu Abbruch und Aufräumen temporärer
Schreibdateien. Originaldatei und offener Entwurf bleiben erhalten; ein fehlgeschlagenes
Speichern im Wechsel-Dialog erhält auch die ausstehende Entscheidung.

Die Prüfung ist keine prozessübergreifende Transaktion. Andere Prozesse können
nach der letzten Prüfung erneut schreiben; Dateien und Metadaten sind nicht
zusammen gesperrt. Eine während einer begonnenen Erstellung angelegte leere
Unterstruktur kann bei Fehler bestehen bleiben. Vorhandene Dateien werden dadurch
nicht ersetzt. Der Unterschied zwischen erkanntem Konflikt und einer globalen
Transaktionsgarantie gehört zum Vertrag, nicht in eine Erfolgsaussage der UI.

`live-metadata-guard` prüft unbekanntes Schema, defekte und fehlende Metadaten mit
bereits geöffnetem Projekt, Kopieren, Erstellen, Archivieren, Save-Guard und
Wiederkehr gültiger Daten. Die neue Regression schlägt mit dem bisherigen Kern
beim ersten Speicherversuch fehl. Der Parser und alle Prüfungen bleiben eigenes C.


Abschließender lokaler Nachweis: 22 Release-Tests bestehen (302,43 Sekunden),
zusätzlich die ergänzte UI-Nachprüfung (14,47 Sekunden). Elf reine Kerntests
bestehen (2,41 Sekunden). Fünf gezielte ASan/UBSan-Wege bestehen (8,45 Sekunden),
die ergänzte UI-/Guard-Nachprüfung ebenfalls (22,27 Sekunden). Die Regression
enthält 75 Guard-Aussagen; die UI prüft Save-Verweigerung und Wiederkehr gültiger
Metadaten innerhalb ihrer 47 Aussagen. Das tatsächliche Fehlerbild wurde betrachtet.
Mit dem bisherigen Kern schlägt die Guard-Regression beim Speichern fehl (Exit 1).
Native Plattform- und entpackte Paketabnahme zu 0.7.3 folgen gesondert.
