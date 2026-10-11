# Hervorhebungen und Links wiederverwenden

Stand: 11. Oktober 2026. Umsetzung ab 0.9.56, vollständige Release-Abnahme
noch offen. Die Änderung folgt der [erweiterten Frame-Messung](FRAME_MESSUNG.md).

## Gemessene Wiederholung

Zwei zusätzliche Metal-Proben teilen die Readerarbeit auf. Bei der langen
Übergabe vom 7. Oktober werden in 68 Scroll- beziehungsweise Kamerabildern
10.132 Absatz-/Überschriftenanalysen ausgeführt. Das erneute Erkennen von
Hervorhebungen, Code und Verweisen kostet darin etwa 183–199 ms insgesamt,
also 2,7–2,9 ms pro Bild. Native Blockausgaben und deren Veröffentlichung
bleiben weitere Kosten. Die verschachtelten Messbereiche dürfen nicht als
unabhängige Gesamtsumme addiert werden.

Originale: `build/reader-stages-verification/`, insbesondere eingefrorene
Diagnosequellen, `build-probe.py`, `probe-1.log`, `probe-2.log` und CSV-Dateien.
Die zusätzlichen Timer haben eigenen Aufwand; sie lokalisieren die Arbeit,
ersetzen aber keinen Leistungsvergleich der unveränderten Produktionspfade.

## Bindung und Speicherbesitz

Der bereits gehaltene Vordergrund-Dokumentbaum besitzt jetzt bei Bedarf auch
die Inline-Analyse und den aufbereiteten Text seiner Absätze und Überschriften.
Ein Eintrag gehört zu einem konkreten Knoten und dessen vollständigem Inhalt.
Verwendung erfordert dessen gehaltene Textadresse, Länge und Referenzumgebung.
Die übergeordnete Quelle wird weiterhin vollständig nach Textbytes verglichen;
Textänderungen ersetzen den gesamten Plan einschließlich aller Inline-Einträge.

Schrift, Breite und Scrollposition verändern den Quellenplan nicht. Aktuelle
Zeichnung, Zeichenrechtecke und Linkaktionen folgen weiterhin der jeweiligen
Ansicht. Der Linkiterator erhält pro Verwendung eine eigene Cursorposition;
der gehaltene Eintrag wird beim Durchlaufen nicht verändert. Seine Referenzen
zeigen auf den Besitzer, nicht auf eine kurzlebige Kopie im aktuellen Frame.
Hintergrunddokumente ersetzen diese Einträge nicht.

Alle Einträge teilen das bisherige 16-MiB-Budget des Dokumentplans. Gezählt
werden die Knotenplätze, Eintragsstrukturen, alle Kapazitäten der Inline-Arrays,
Textspan-Kapazitäten und eine exakt große Kopie des ausgegebenen Texts. Der
eigene C-Leser veröffentlicht dafür die Größe seiner gehaltenen Arrays mit
`sb_inline_storage_bytes`; bei Größenüberlauf meldet er `SIZE_MAX`.
Geliehene Quelle und Referenzen werden im Dokumentplan bereits gezählt.

Wenn Platz oder Speicher fehlen, bleibt die vollständige bisherige Analyse
als kurzlebiges Ergebnis erhalten. Das Budget begrenzt gehaltenen Speicher,
nicht die vorübergehende Parser-Spitze. Tabellenzellen und die Erfassung der
Worker-Aufträge verwenden weiterhin ihren bisherigen Inline-Pfad. Die erste
Aufbereitung und die vollständige Leistungsaufgabe sind damit nicht erledigt.

Beim Ersetzen eines ausgeliehenen Plans werden die Ausgabestrukturen außerdem
vor der neuen Analyse zurückgesetzt. Ein neuer Fehlerfall im Prüfer zeigte,
dass sonst nach einem frühen Parserfehler bereits freigegebene Referenzen in
die Bereinigung gelangen konnten. Die produktive Framefunktion verwendet
frische Ausgaben; der neue Test prüft zusätzlich die wiederholte Übergabe
geliehener Ausgaben an denselben internen Helfer.

## Geprüfter Teilumfang

56 Quellprüfungen erfassen eigenen Speicher, unveränderte Wiederverwendung,
Cursor-Neustart, geänderte Referenzen bei gleicher Textlänge, Hintergrundtexte,
Unicode/Hervorhebungen/Code/Entities, Speicherbudget und vollständigen Rückfall.
Ein Absatz mit 1.000 Links überschreitet das kleine Testbudget; alle Links
bleiben verwendbar. Vergleichstext und Stilfelder stammen aus einer frisch
erzeugten Analyse. Struktur-Padding wird dabei nicht verglichen.

49 produktive Readerprüfungen bestätigen Wiederverwendung ohne veränderte
native Geometrie sowie Resize, Schriftwechsel, Bearbeiten und schnellen
Dokumentwechsel. Reader-Geometrie, native Mac-Anbindung und Interaktionsprüfung
bestehen im gezielten Release-Lauf. Quell-/Reader- und 621 Mac-Assertions
bestehen auch unter haltendem ASan/UBSan: 39 eigene C-Dateien, 55 aktuelle
Quell-/Include-Hashes. UI-Abhängigkeiten sind nicht instrumentiert; keine
Leak-/ThreadSanitizer- oder menschliche Screenreader-Abnahme.

Originale: `build/reader-inline-verification/` und
`build/reader-inline-sanitizers/`. 78/78 lokale Release-Prüfungen bestehen
(552,41 s). Paket- und Installationsnachweise werden nach Abschluss im
[Umsetzungsstand](STATUS.md) ergänzt; installiert ist noch 0.9.55.

## Alternierende Metal-Messung

Drei serielle Paare vergleichen die eingefrorene Desktop-/Quellimplementierung
aus `be6c1e2` mit der neuen Fassung. Beide verwenden denselben aktuellen
Timingprüfer und unveränderten UI-Bibliotheksstand. Der additive Kernhelfer
verändert die bisherigen Parserfunktionen nicht. Die gemeinsame öffentliche
Desktop-Struktur enthält auf beiden Seiten die zusätzlichen Diagnosezähler.
Die 18 Dateien der Profilkopie bleiben unverändert; kein gleichzeitiger Build
oder weiterer UI-Prüfer. macOS 14.6.1/23G93, Intel-Mac, Metal, 1336×840.

| Paar | Scroll-Layout vorher → danach | Kamera-Layout vorher → danach | Wechsel-Layout vorher → danach | Wechsel-Frame-p95 vorher → danach |
| --- | ---: | ---: | ---: | ---: |
| 1 | 10,43 → 7,55 ms | 10,64 → 7,48 ms | 2,45 → 2,06 ms | 33,39 → 25,45 ms |
| 2 | 12,80 → 7,88 ms | 13,43 → 7,56 ms | 2,97 → 2,06 ms | 44,42 → 28,80 ms |
| 3 | 14,15 → 9,74 ms | 14,45 → 9,31 ms | 2,45 → 1,90 ms | 33,50 → 30,61 ms |

Median der paarweisen Layoutverbesserung: 31,20% beim Scrollen, 35,54% bei
Kamerabewegung, 22,63% bei Wechseln. Die gesamte Frame-p95 verbessert sich
bei Scroll/Kamera nicht in jedem Paar. Das erste Bild liegt in der neuen
Fassung bei 92–119 ms; im dritten Paar bleibt seine Gesamtdauer praktisch
gleich. Systemlast, GPU-Wartezeiten und die erste Vorbereitung bleiben relevant.
Dies ist kein allgemeiner Flüssigkeitsnachweis.

Originale: `paired-profiles.json`, sechs CSV-/Textprotokolle und
`baseline/` unter `build/reader-inline-verification/`.
Aktuelle Windows-/Linux-/ARM64-, Geräte-, Langzeit-, Eingabe- und assistive
Abnahmen bleiben offen. SBUI-074 und 1.0 sind nicht freigegeben.
