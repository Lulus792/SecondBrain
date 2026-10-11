# SecondBrain: aktueller Stand

Stand: 11. Oktober 2026. Originale: [STATUS](../../docs/STATUS.md),
[Release-Liste](../../docs/RELEASE.md), [Quellen](SOURCES.md).

## Produkt und installierte App

Eigene C17-App für Mensch und KI: Projekte/Notizen anlegen, lesen, bearbeiten,
suchen, archivieren, sichern; Quellen und gespeicherten Kontext kopieren.
Lumen-Sternkarte, Glaskarten, Icons, direkte Pfeilnavigation, Kamerafahrt,
Scrollen, Startfokus, Textcursor und große Leseansicht sind implementiert.
Originalbytes, Entwürfe und erkannte Konflikte bleiben geschützt.

Installiert weiterhin **0.9.44 / 0455e5a276f8**; Versionsaufruf am 11. Oktober
bestätigt. Die früheren lokalen Build-/Paket-/Diagnoseverzeichnisse sind nicht
mehr vorhanden. Historische Abnahmen bleiben in STATUS und im Journal erhalten;
ihre entfernten Dateien sind keine aktuell zugänglichen Prüfbelege.
Ein frischer Release-Build liegt unter `build/current-ui`.

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
70/70 lokale Release-Prüfungen bestehen (488,53s). Paket/Installation und
Zielplattformen folgen.

## Nächste Arbeiten

Die aktuelle Gesamt-/Paketprüfung abschließen, geprüften Schritt veröffentlichen
und die Mac-App aktualisieren. Danach Quelllesen, Parser/Inline-Aufbereitung,
native Metadaten, Rasterung und Fälle über den Vorbereitungsbudgets profilieren.
Der Rückfall ist noch synchron; einzelne Bibliotheksaufrufe und Shutdown können
warten. SBUI-074 bleibt für die vollständige Großdaten-/Geräteabnahme offen.

Weitere native Textfelder/Leseblöcke, echte IME und VoiceOver/NVDA/Orca,
Dialog-/Geräte-/Monitor-/Langzeitprüfungen, Windows-/Linux-Speicherfehler,
Mindest-OS/frische Rechner, vollständige Runtime-/Lizenzprüfung, Sicherheitskanal,
Signierung und abschließende natürliche Texte bleiben Release-Arbeit.
**1.0 nur nach ausdrücklicher Nutzerfreigabe.**
