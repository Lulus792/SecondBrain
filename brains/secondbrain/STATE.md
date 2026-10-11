# SecondBrain: aktueller Stand

Stand: 11. Oktober 2026. Originale: [STATUS](../../docs/STATUS.md),
[Release-Liste](../../docs/RELEASE.md), [Quellen](SOURCES.md).

## Produkt und installierte App

Eigene C17-App für Mensch und KI: Projekte/Notizen anlegen, lesen, bearbeiten,
suchen, archivieren, sichern; Quellen und gespeicherten Kontext kopieren.
Lumen-Sternkarte, Glaskarten, Icons, direkte Pfeilnavigation, Kamerafahrt,
Scrollen, Startfokus, Textcursor und große Leseansicht sind implementiert.
Originalbytes, Entwürfe und erkannte Konflikte bleiben geschützt.

Installiert **0.9.55 / 11c9cb4f1e99**, sauberer Intel/macOS-Release-Build.
96 Dateien entsprechen dem geprüften Paket; 18 Gedächtnisdateien beim Laden
unverändert. 126 Desktop-, 161 Tastatur-, 75 Sicherungs-Assertions, zwei Neustarts,
produktive CLI und Runtime-/Lizenz-/Version bestehen.
Rückfallversion 0.9.54: `/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.54-20261011-165023`.
CI 38148508446 zum Quellcommit zuletzt queued; neue Gesamt-Plattformabnahme offen.

Frühere Build-/Paket-/Diagnoseverzeichnisse waren am11.Oktober nicht mehr
vorhanden. Historische Abnahmen bleiben in STATUS/Journal; entfernte Dateien
sind keine aktuell zugänglichen Prüfbelege. Neuer Build: `build/current-ui`.

## Aktuelle Arbeit (0.9.46)

Die Vorbereitung formatierter Texte aus 0.9.44/45 ist jetzt an den Reader
angeschlossen: ein privater Fontkontext/Thread pro Dokument, genaue Block-/
Tabellenbreiten und gehaltene Pläne außerhalb des allgemeinen Memo-Caches.
Navigation bricht ohne laufenden Join ab; übernommen wird die letzte passende
Auswahl. Resize erhält währenddessen die logische Scrollposition. Modale Reader
besitzen den Auftrag gegenüber ihrem verdeckten Dokument. Ein aus einer Quelle
zurückkehrender Linkfokus wird nach der Vorbereitung wiederhergestellt.
Aktuelle Hover-/Tastaturhinweise unterbrechen eine überlagernde Überblendung.
[Vertrag und Grenzen](../../docs/READER_VORBEREITUNG.md).

3250 Schrift-/Job-/Batchprüfungen und 44 produktive Readerprüfungen bestehen
auch unter haltendem ASan/UBSan im dokumentierten Teilumfang. Native
Blockgeometrie/Scrollmaximum stimmen mit dem synchronen Referenzpfad überein.
Interaktions- und Tastaturprüfung bestehen nach den Anschlusskorrekturen.
70/70 lokale Release-Prüfungen bestehen (488,53s). Intel/macOS-Paket und
Installation bestehen; weitere Zielplattformen folgen.

## Neue Ebenen-/Fokuspolitur (0.9.47)

Sternkarten-Fokus/Namen scheinen nicht mehr durch Karten, Liste und Kameraleiste.
Verdeckte Mapziele und die Map eines offenen Editors sind keine Tabstopps;
F6 zeigt sie wieder. Such-/Dialogrückkehr erhält sichtbaren Fokus. Die Karte
öffnet sich erst nach akzeptiertem Notizwechsel, ohne den alten Editor vor
einer Entwurfsentscheidung einzublenden. [Vertrag](../../docs/UI_EBENEN.md).
135 neue Ebenenprüfungen auch unter ASan/UBSan und der gesamte Keyboardnachlauf
bestehen.71/71 lokale Release-Prüfungen bestehen (626,20s); Intel/macOS-Paket
und Installation geprüft. Weitere Zielplattformen bleiben offen.

## Neue Rasteroptimierung (0.9.48)

Glasinnenflächen werden ohne Randgeometrie und wiederholte Spaltenberechnung
gezeichnet; Rand/Schatten/Reihenfolge bleiben gleich.1012 vollständige Pixel-
vergleiche gegen0.9.47 auch unter ASan/UBSan bestehen. Drei alternierende Metal-
Paare verbessern Kamera-Zeichenzeit im Median um19,36%; Profilkopie unverändert.
Die tatsächlichen Frames haben weiterhin Layout-/Presentspitzen.
[Vertrag/Messung](../../docs/RASTER_LEISTUNG.md).72/72 lokale Release-Prüfungen
bestehen (395,24s). Intel/macOS-Paket und Installation geprüft; andere
Zielplattformen bleiben offen.

## Neue Feldgeometrie (0.9.49)

Suche und alle Formulartexte kopieren ihre tatsächliche gezeichnete Geometrie
mit eigenen Quellen; IME und der gehaltene Notizeditorcache bleiben geschützt.
503 Feld- und 592 Mac-Providerprüfungen bestehen auch unter ASan/UBSan.
72/73 Release-Prüfungen plus korrigierter zeitabhängiger Cursor-Test im
gezielten Nachlauf decken alle73 aktuellen Prüfungen ab. Mac-Paket und Installation
bestehen; andere Zielplattformen bleiben offen.
[Vertrag/Prüfgrenzen](../../docs/NATIVE_TEXTGEOMETRIE.md).

## Neue Feldaktionen (0.9.50)

Suche/Formulare bieten native Auswahl/Ersetzung mit Graphemgrenzen, atomischer
Kapazitätsprüfung und erhaltener Fokusrückkehr. Der Mac-Anschluss ergänzt den
fehlenden Setter auch im Notizeditor; readonly/NUL und getrennte Fensterqueues
sind geprüft.74/74 Release-Prüfungen bestehen713,91s;190 Feldaktions-,503
Geometrie- und608 Provider-Assertions auch unter ASan/UBSan im dokumentierten
Umfang. Mac-Paket und Installation bestehen; andere Zielplattformen bleiben offen.
[Vertrag](../../docs/NATIVE_FELDAKTIONEN.md).

## Neue Lesegeometrie (0.9.51)

Textblöcke/Code und gezeichnete Tabellenzellen übernehmen ihre tatsächlichen
Zeichenrechtecke. Pläne halten native Läufe; private Worker erzeugen sie mit,
32-/64-MiB-Budgets zählen die Kapazität.75/75 Release-Prüfungen bestehen591,50s;
590 Reader-,5231 Layout-/Batch- und616 Mac-Provider-Assertions auch unter
ASan/UBSan im dokumentierten Umfang. Drei Metal-Paare zeigen keinen großen
neuen Einbruch, aber weiterhin Ausreißer; keine allgemeine Flüssigkeitsaussage.
Mac-Paket und Installation bestehen; andere Zielplattformen bleiben offen.
[Vertrag/Messung](../../docs/NATIVE_LESEGEOMETRIE.md).

## Native Titel und Markierungen (0.9.52)

Textbefehle verbinden sichtbare Titel, Beschriftungen und nummerierte Marker mit
nativen Zeichenflächen; Aufzählungspunkte melden ihre tatsächliche Kreisfläche.
Vollständige Namen bleiben erhalten, dekorative Titel stehen außerhalb des
Dokumenttexts. 76/76 Release-Prüfungen bestehen (624,51 s); 354 Titel-, 590 Reader-
und 621 Mac-Provider-Assertions auch unter ASan/UBSan im dokumentierten Umfang.
39 eigene C-Dateien instrumentiert; 53 Quell-/Include-Hashes stimmen.
Mac-Paket und Installation bestehen; andere Zielplattformen bleiben offen.
[Vertrag](../../docs/NATIVE_TITELGEOMETRIE.md).

## Reader-Quellplan (0.9.53)

Unveränderte Vordergrunddokumente behalten ihren eigenen Baum und Referenzen,
gebunden an vollständige Textbytes. 16 MiB begrenzen gehaltene Kapazitäten;
Editorwechsel gibt den Plan frei, übergroße Pläne bleiben im vollständigen
synchronen Pfad. 22 Quell-, 48 produktive Reader-, 590 Geometrie- und 621
Mac-Providerprüfungen bestehen; Quell-/Reader-/Provider auch unter ASan/UBSan.
39 eigene C-Dateien instrumentiert, 54 Quell-/Include-Hashes passen.
Drei Metal-Paare senken Wechsel-Layoutzeit; Bildausgabespitzen bleiben.
77/77 lokale Release-Prüfungen bestehen (539,97 s). Mac-Paket und Installation bestehen; andere Zielplattformen bleiben offen.
[Vertrag und genaue Grenzen](../../docs/READER_QUELLPLAN.md).

## Aktionsicons und Export (0.9.54)

SBUI-076/078: Reloadbögen, Sicherungsicon und regenerativer Export erhalten
vorhandene Zuordnungen/Punktflächen. 136 Interaktions-Assertions vorher/nachher,
100/200%-Aufnahmen, fünf gezielte Prüfungen und Python unittest bestehen;
31 Exportdateien reproduzierbar. Mac-Paket/Installation bestehen; weitere Zielplattformen offen.
SBUI-077 bleibt offen: konkurrierende Dialog-Hintergrundtexte beruhigen und
Glascharakter erhalten. [Bericht](/Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-11_15-36-09/UI_REVIEW.md).

## Dialogmaterial (0.9.55)

SBUI-077 im lokalen Pixel-/Darstellungsumfang nachgeprüft: aktuelle Glasfläche
vor eigenem Dialoginhalt, keine fremden Beschriftungen.352/136/621 Material-/
Interaktions-/Provider-Assertions auch unter ASan/UBSan,39 C-Dateien,54 aktuelle
Hashes. Vier gültige Metalbilder angesehen; Aufnahme-/Snapshot-Grenzen dokumentiert.
78/78 lokale Release-Prüfungen bestehen (546,80 s). Mac-Paket/Installation bestehen; weitere Zielplattformen offen.
[Vertrag](../../docs/DIALOGMATERIAL.md), [Bericht](/Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-11_16-38-09/UI_REVIEW.md).

## Nächste Arbeiten

0.9.56 hält Inline-Analysen von Absätzen/Überschriften im vorhandenen 16-MiB-
Quellbudget. Drei serielle Metal-Paare verbessern Layout beim Scrollen/Kamera
im Median um 31,20/35,54%; erste Vorbereitung und Frame-Spitzen bleiben.
56 Quell-, 49 Reader- und 621 Mac-Assertions auch unter ASan/UBSan; 39 eigene
C-Dateien, 55 aktuelle Hashes. 78/78 Release-Prüfungen bestehen (552,41 s).
Paket/Installation folgen; installiert weiterhin 0.9.55.
[Vertrag und Grenzen](../../docs/READER_INLINE_PLAN.md).

Der Diagnoseprüfer erfasst jetzt auch Eingabe und erste Bilder, alle 272
Framewerte und reale Animationsabstände. Drei serielle Metal-Läufe zeigen
114–126 ms Arbeit im ersten Bild der langen Notiz; Bewegungs-/Zeichenspitzen
bleiben. CSV-/Prozess-/Bestandsschutzprüfung besteht. Keine Produktoptimierung
oder neue Version; installiert bleibt 0.9.55.
[Messung und Grenzen](../../docs/FRAME_MESSUNG.md).

Aktuelle Dialog-/Geräteabnahme, Quelllesen, Parser/Inline-Aufbereitung,
native Metadaten, Rasterung und Fälle über den Vorbereitungsbudgets profilieren.
Der Rückfall ist noch synchron; einzelne Bibliotheksaufrufe und Shutdown können
warten. SBUI-074 bleibt für die vollständige Großdaten-/Geräteabnahme offen.

Aktuelle Plattformabnahme, echte IME und VoiceOver/NVDA/Orca,
Dialog-/Geräte-/Monitor-/Langzeitprüfungen, Windows-/Linux-Speicherfehler,
Mindest-OS/frische Rechner, vollständige Runtime-/Lizenzprüfung, Sicherheitskanal,
Signierung und abschließende natürliche Texte bleiben Release-Arbeit.
**1.0 nur nach ausdrücklicher Nutzerfreigabe.**
