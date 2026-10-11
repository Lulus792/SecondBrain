# SecondBrain: aktueller Stand

Stand: 11. Oktober 2026. Originale: [STATUS](../../docs/STATUS.md),
[Release-Liste](../../docs/RELEASE.md), [Quellen](SOURCES.md).

## Produkt und installierte App

Eigene C17-App für Mensch und KI: Projekte/Notizen anlegen, lesen, bearbeiten,
suchen, archivieren, sichern; Quellen und gespeicherten Kontext kopieren.
Lumen-Sternkarte, Glaskarten, Icons, direkte Pfeilnavigation, Kamerafahrt,
Scrollen, Startfokus, Textcursor und große Leseansicht sind implementiert.
Originalbytes, Entwürfe und erkannte Konflikte bleiben geschützt.

Installiert **0.9.49 / 5a4aee460031**, sauberer Intel/macOS-Release-Build.
95 Dateien entsprechen dem geprüften Paket;18 Gedächtnisdateien beim Laden
unverändert.126 Desktop-,161 Tastatur-,75 Sicherungs-Assertions, zwei Neustarts,
produktive CLI und Runtime-/Lizenz-/Version bestehen.
Rückfallversion0.9.48: `/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.48-20261011-124433`.
CI38132768614 zum Quellcommit zuletzt queued; neue Gesamt-Plattformabnahme offen.

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

## Nächste Arbeiten

Quelllesen, Parser/Inline-Aufbereitung,
native Metadaten, Rasterung und Fälle über den Vorbereitungsbudgets profilieren.
Der Rückfall ist noch synchron; einzelne Bibliotheksaufrufe und Shutdown können
warten. SBUI-074 bleibt für die vollständige Großdaten-/Geräteabnahme offen.

Native Leseblock-Geometrie und einzeilige Auswahl-/Ersetzungsaktionen, echte IME und VoiceOver/NVDA/Orca,
Dialog-/Geräte-/Monitor-/Langzeitprüfungen, Windows-/Linux-Speicherfehler,
Mindest-OS/frische Rechner, vollständige Runtime-/Lizenzprüfung, Sicherheitskanal,
Signierung und abschließende natürliche Texte bleiben Release-Arbeit.
**1.0 nur nach ausdrücklicher Nutzerfreigabe.**
