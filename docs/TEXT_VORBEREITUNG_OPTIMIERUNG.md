# Weniger Arbeit beim vorbereiteten Textlayout

Stand: 12. Oktober 2026. Entwicklungsstand 0.9.66, noch nicht als 1.0 freigegeben.
Der Nutzer meldete verzögerte Wechsel zwischen Eingang, Einstieg und der langen
Übergabe vom 7. Oktober. Die [Frame-Messung](FRAME_MESSUNG.md) trennt dafür
angenommene Auswahl, vorbereitete Textkommandos und Ende des Übergangs.

## Änderung

Bei der Suche nach einer passenden Zeilengrenze wird derselbe Text weiterhin
kontextuell mit HarfBuzz geformt. Diese Versuche benötigen nur die genaue
Vorschubbreite. Sie legen deshalb keine Glyphenliste an und laden keine
Glyphenrechtecke aus FreeType. Schreibrichtung, Schrift- und Stilgrenzen,
Spiegelung, Sprache, Kerning und synthetischer Fettdruck verwenden denselben
Pfad wie die vollständige Formung. Die tatsächlich gewählte Zeile erhält
weiterhin ihre vollständige Geometrie für Darstellung und native Textrechtecke.
Das ist keine Schätzung aus einzelnen Zeichenbreiten.

Identische Vorbereitungsaufträge innerhalb eines Dokumentbatches besitzen nun
nur einen Text-/Stilplan. Gleichheit umfasst den gesamten Text, Stilspannen,
Schriftrolle, Breite, Abstand, beide Innenabstände und die Abschlusszeile.
Hashwerte dienen nur der Suche; vollständiger Vergleich entscheidet.
Die vorhandenen Grenzen von 4096 Aufträgen und 32 MiB angebotenem Inhalt gelten
weiterhin für die ursprüngliche Eingabe, einschließlich Wiederholungen.

Die Erweiterungen liegen im eigenen C-Code und der eigenen MIT-Anbindung
`third_party/ui/ttf_shape.inc`. Es wurde keine fachliche Bibliothek hinzugefügt.

## Gemessener Umfang auf Intel/macOS 14.6.1

Sechs serielle Metal-Läufe, 1336 × 840, jeweils 272 Bilder, Abstand 24 Bilder.
Die Reihenfolge war optimiert, vorherig, vorherig, optimiert, optimiert, vorherig.
Beide eingefrorenen Prüfer verwenden denselben Diagnosequelltext. Alle 18 Dateien
der Arbeitskopie und des originalen Gedächtnisses blieben bytegleich.

| Paar | Lange Notiz vorbereitet, vorher → nachher | Ohne Übergang, vorher → nachher |
| --- | ---: | ---: |
| 1 | 211,87 → 189,54 ms | 376,78 → 351,78 ms |
| 2 | 214,30 → 185,34 ms | 382,79 → 344,48 ms |
| 3 | 215,38 → 207,61 ms | 387,42 → 373,83 ms |

Die medianen paarweisen Verbesserungen betragen 10,54 % beziehungsweise 6,64 %.
Dies sind interne erfolgreiche Renderer-Submissionen ab der Notizanforderung,
keine Messung der sichtbaren Bildschirmabtastung oder menschlichen Lesbarkeit.
Systemlast, Schrift-/Dateicaches und das künstliche Framebudget beeinflussen sie.
Kleine Notizen waren nicht durchgehend schneller. Frühere Messungen unter anderer
Last sind kein direkter Vergleich zu dieser Tabelle.

Die lange Notiz hat 149 verschiedene Vorbereitungsaufträge. Deduplizierung
verringert ihren Umfang deshalb nicht und hat allein keinen verlässlichen
Zeitgewinn gezeigt. Ihr belegter Nutzen ist Speicher: 128 gleiche Testabsätze
benötigen 60.547 statt 1.046.484 Bytes für 128 verschiedene Absätze desselben
Umfangs, rund 94,2 % weniger in diesem gezielten Fixture.

## Prüfung und offene Grenzen

Ein Referenzmodus führt die Umbruchversuche weiterhin mit vollständiger
Glyphengeometrie aus. Die Prüfung vergleicht den Hintergrundplan dagegen:
Zeilenhöhen, Grenzen, Glyphen, Positionen, Vorschübe, Rechtecke, Stil-/Bidi-Läufe
und native Zeichenrechtecke. Insgesamt 24 Kombinationen aus Unicode, Rollen, Breiten und
Abschlusszeilen bei drei Schriftgrößen sind abgedeckt. Weitere Prüfungen
vergleichen alle Graphem-Teilbereiche gemischter arabischer/hebräischer Absätze,
Spiegelung und Rastermetriken. Ungültige Eingaben erhalten den Ausgabewert.
Die Wiederholungsprüfung wurde vor der Änderung tatsächlich negativ ausgeführt;
Layoutvarianten bleiben getrennt und die ursprüngliche Eingabegrenze bleibt wirksam.

Die drei gezielten Release-Prüfungen für Glyphengeometrie, Hintergrundvorbereitung
und Reader bestehen (8,39 s). Alle 83 lokalen Release-CTest-Prüfungen bestehen (summierte Testzeit 504,86 s).
ASan/UBSan bestehen für 5385 Vorbereitungs-, 120 Reader- und 1019 Glyphenbedingungen.
38 eigene C-Dateien und 58 aktuelle Quell-/Includehashes wurden bestätigt.
Externe UI-Bibliotheken sind nicht instrumentiert; Leakerkennung ist aus, kein TSan.
Das saubere Intel/macOS-Paket 0.9.66/b3b93709f823 besteht 126 Desktop-,
167 Tastatur- und 75 Sicherungsbedingungen, zwei Einstellungsprozesse sowie
CLI-/Laufzeit-/Lizenz-/Versionsprüfung. Die Paketwiederöffnung erhält 20 Dateien.
Der echte Wechsel 0.9.65 → 0.9.66 → 0.9.65 erhält diese Dateien und Einstellungen;
Sicherung/Wiederherstellung bestehen. Alle drei App-Aufnahmen wurden angesehen.
98 Dateien paketgleich unter `dist/SecondBrain` installiert; 18 eigene
Gedächtnisdateien beim Öffnen unverändert. Installierte App-Aufnahme angesehen.
Die Binärdatei deklariert macOS 14.0, SDK 15.2; ausgeführt auf 14.6.1/23G93,
x86_64. Dies ist keine ausgeführte Abnahme auf 14.0 oder ARM64.

Archive SHA256: `0bb4974d8d9e56d0355e78ec0095ccaacb2a0efc3402334770aa970421d42ef0`.
Rückfallkopie: `/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.65-20261012-001235`.
Originale: `build/text-optimization-package-check`, `build/text-optimization-update`,
`build/text-optimization-verification/installed.json` und
`installed-brain-check.json`. Fünf abschließende Prüfungen am sauberen Build
für Glyphen, Vorbereitung, Reader und Versionsidentität bestehen (8,87 s). Schwächere Geräte, Trackpads, Windows/Linux/ARM64,
menschliche Bedienung und große reale Wissensbasen bleiben gesonderte Abnahmen.
SBUI-074 ist weiter offen; diese Verbesserung erfüllt nicht die gesamte Release-Liste.

Originale: `build/batch-dedup-verification/optimization-evidence.json`,
`width-results.json`, sechs `width-*.csv/log`, `baseline.json` und eingefrorene
Prüfer. Baseline SHA256: `21d4163317498240fe1b643a630bb3881af27431519a8069e6d3cd6d426576ee`;
optimiert: `2d96e3c33130e3e51926d9c316f1dd355ee5c62bacd9225490c2ff1f81fdcc38`.
