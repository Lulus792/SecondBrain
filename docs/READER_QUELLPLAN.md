# Wiederverwendung des Reader-Dokumentbaums

Stand: 11. Oktober 2026, Umsetzung ab 0.9.53.

Die Leseansicht musste bisher für jedes Bild denselben Markdown-Baum und dessen
Referenzdefinitionen erneut erzeugen. Sie hält jetzt einen eigenen Quellplan
für das aktive Dokument. Die Quelle wird vollständig kopiert; Dokumentansichten
und Referenzen leihen ausschließlich diese gehaltenen Bytes. Der gezeichnete
Text, Umbruch, Linkaktionen und die native Dokumentstruktur behalten ihre Regeln.

## Gültigkeit und Speicher

Wiederverwendung erfordert gleiche Länge und identische Textbytes. Auch eine
Änderung innerhalb desselben Eingabepuffers ersetzt den gesamten Plan. Gleiche
Bytes können den strukturellen Plan unabhängig von Schrift, Breite und Scrollen
verwenden; aktuelle Darstellung und Linkauflösung folgen weiterhin der aktiven
Ansicht und dem aktiven Projekt. Es werden keine Bildschirmrechtecke gespeichert.

Nur der Vordergrundreader besitzt diesen Plan. Ein verdecktes Dokument darf ihn
nicht durch seine Referenzen ersetzen. Beim Wechsel zum Editor oder einer Ansicht
ohne formatierten Reader wird er freigegeben. Der Textlayout-Worker besitzt seine
separaten kopierten Anforderungen und benötigt keine geliehenen Parseransichten.

Der gehaltene Plan ist auf 16 MiB begrenzt. Gezählt werden Quellbytes,
Dokumentknoten-Kapazität, alle Projektionspuffer und Quellabbildungen sowie
Referenzkapazität und normalisierte Namen. Bei Speicherfehlern oder zu großer
Kapazität bleibt der vollständige bisherige Parserpfad erhalten. Ein übergroßer
Plan wird nicht dauerhaft gespeichert; die erste Aufbereitung und dieser Rückfall
bleiben synchron. Das Budget begrenzt den gehaltenen Plan, nicht die temporäre
Speicherspitze beim Parsen oder den gesamten Speicher der Anwendung.

## Prüfungen und Messung

22 Quellprüfungen testen eigenen Speicher, unveränderte Wiederverwendung,
Änderungen gleicher Länge, neue Linkdefinitionen, getrennte Hintergrundquellen,
Budgetüberschreitung mit vollständigem Inhalt, ungültiges UTF-8 und leere Quellen.
Die identische Budgetprüfung verwendet im Prüfer 64 KiB, um den Rückfall ohne
riesige Testdatei auszulösen. 48 produktive Readerprüfungen erfassen zusätzlich
unveränderte Geometrie, Größenwechsel und Freigabe beim Bearbeiten.
590 Reader-Geometrie- und 621 native Mac-Provider-Assertions bestehen lokal.
Quell-, produktive Reader- und Providerprüfungen bestehen auch unter haltendem
ASan/UBSan: 39 eigene C-Dateien, 54 übereinstimmende Quell-/Include-Hashes.
UI-Abhängigkeiten sind nicht instrumentiert; kein Leak- oder ThreadSanitizer-Nachweis.

Drei abwechselnde Metal-Paare vergleichen den eingefrorenen Stand 0.9.52 mit der
neuen Fassung, beginnend mit der langen Übergabe. Die 18 Dateien der Profilkopie
bleiben unverändert. Durchschnittliche Layoutzeit beim Wechsel:

| Paar | Vorher | Danach | Frame-p95 vorher | Frame-p95 danach |
| --- | ---: | ---: | ---: | ---: |
| 1 | 4,18 ms | 2,14 ms | 48,22 ms | 26,46 ms |
| 2 | 2,57 ms | 2,37 ms | 29,11 ms | 25,31 ms |
| 3 | 2,67 ms | 1,91 ms | 32,17 ms | 32,94 ms |

Median der paarweisen Layoutverbesserung: 28,46 Prozent. Bildausgabe und Systemlast
schwanken; Scroll-p95 verbessert sich nicht in jedem Paar. Die vollständige
Leistungsaufgabe bleibt offen. Eine frühere Messreihe lief teilweise gleichzeitig
mit dem Sanitizerbuild und gilt nicht als isolierter Leistungsvergleich.

Eine zuvor untersuchte Farbtabelle für flache Glasflächen bestand 1012 vollständige
Pixelprüfungen, brachte aber keinen verlässlichen Gewinn in den drei Metal-Paaren.
Diese Änderung wurde entfernt. Die vorhandene Glasdarstellung bleibt erhalten.

Originale: `build/reader-source-verification/` mit eingefrorener Referenz,
Pixelprüfung, erster Messreihe und `source-cache-serial/paired-profiles.json`.
Speicherprüfungen: `build/reader-source-sanitizers/`.
77/77 lokale Release-Prüfungen bestehen (539,97 s). Das Intel/macOS-Paket ist
geprüft und als 0.9.53 installiert; neue Zielplattformabnahmen bleiben offen. Tatsächliche Geräte-, Screenreader-, Eingabe-
und Langzeitabnahmen sowie der vollständige 1.0-Auftrag bleiben offen.

Ab 0.9.56 teilen gehaltene [Inline-Analysen](READER_INLINE_PLAN.md) von Absätzen
und Überschriften dieses Budget. Ihre Kapazitäten und Knotenplätze werden
mitgezählt. Bei einem fehlgeschlagenen Quellenersatz werden vorangegangene
geliehene Ausgaben vor der neuen Parserarbeit zurückgesetzt; alte Referenzen
gelangen nicht in die Bereinigung des neuen Ergebnisses. Neue Nachweise und
die gesonderten Grenzen stehen im Inline-Vertrag.
