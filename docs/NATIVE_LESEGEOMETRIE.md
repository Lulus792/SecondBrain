# Zeichenrechtecke in formatierten Leseblöcken

Stand: 11. Oktober 2026, Umsetzung ab0.9.51.

Absätze, sichtbare Überschriften im Dokumentkörper, Codeblöcke und tatsächlich
gezeichnete Tabellenzellen veröffentlichen native Zeichenpositionen aus ihrem
vorhandenen Glyphenplan. Das gilt auch für Listen-/Zitatinhalte und die große
Leseansicht. Die separate Kartentitelzeile, Listenmarkierungen und logische
Tabellenköpfe ohne eigenes sichtbares Widget brauchen weitere Integration.

## Ein Plan für Zeichnung und native Geometrie

Jede gewählte Zeile erhält Graphemrechtecke aus ihrem tatsächlichen Glyphen- und
Cursorplan. Vorhandene GDEF-Ligaturpositionen werden verwendet; andernfalls gilt
die dokumentierte proportionale Aufteilung. Diese Rechtecke bezeichnen auswählbare
Zeichenbereiche, nicht die engsten gerasterten Glyphenkonturen.

Für den Umbruch ausgeblendete Leerzeichen und Absatztrenner behalten ihre
Quellposition mit Breite null am betreffenden Zeilenende. CRLF bleibt eine Einheit.
Ein tatsächlich vorhandener leerer Schlussabsatz erhält seine eigene Geometrie;
ein rein semantischer Schlusslauf ohne gezeichnete Zeile erhält keine erfundene
Zeichenfläche. Bidi-Level, Zeilen, Stil und auseinanderliegende Flächen teilen
native Läufe. Grapheme über255 UTF-8-Bytes behalten den bestehenden skalaren
Rückfall ohne unzutreffende genaue Rechtecke.

Die nativen Läufe und Unicode-Wortanfänge entstehen einmal mit dem Layoutplan.
Scrollen und Links-/Mitte-/Rechtsausrichtung kopieren die gehaltenen Läufe und
verschieben ihren Ursprung. Ein anderer Export oder eine spätere Cacheverdrängung
verändert diese besessenen Ausgaben nicht. Die Quelle ist der tatsächlich
angezeigte Text nach Inline-Aufbereitung; Markdown-Originale bleiben unverändert.

Der Reader verwendet seine gezeichnete Widgetbreite, einschließlich der separaten
Mess-/Zeichenbreiten in Tabellen. Literalcode übernimmt seinen nativen Codestil,
behält aber den gleichen einfachen Umbruch wie bei der Zeichnung. Verdeckte
Zeichen behalten Koordinaten außerhalb des Viewports; die vorhandene native
Blocknavigation kann die betreffende Stelle sichtbar machen.

## Vorbereitung und Speicher

Die Geometrie wird im privaten Fontkontext der vorhandenen Hintergrundvorbereitung
mit aufgebaut. Sie enthält keine Fontzeiger; die spätere Glyphenbindung verändert
ihre Quellzuordnung nicht. Native Laufarrays werden nach Möglichkeit auf ihre
tatsächliche Anzahl verkleinert. Gehaltene Laufkapazität und Zeichenrechtecke
zählen zum32-MiB-Styled-Cache und64-MiB-Vorbereitungsbudget. Budgetabweichungen
werden vor Übernahme abgewiesen; der bisherige synchrone Rückfall bleibt bestehen.
Die Spitzenallokation einzelner Aufbereitungsaufrufe ist weiterhin kein global
strikter Speicher- oder Flüssigkeitsnachweis.

## Native Abfrage und Prüfung

Der unabhängige Glyphenvergleich verwendet getrennt vorbereitete Latein-/Arabisch-
und Fettschriftspannen. Er prüft Rechtecke bei100/150/200 Prozent Schrift,
allen drei Ausrichtungen und verschobenen Ursprüngen. Produktive Blöcke prüfen
Quellenabdeckung, Code, Listen/Zitate und Tabellen. Die vorhandene Workerprüfung
vergleicht auch sämtliche nativen Läufe zwischen privatem und synchronem Layout.

Die festgelegte AccessKit-Consumer-Quelle bietet Textbereiche für das Dokument,
aber nicht den Absatzcontainer. Der echte Mac-Test findet den Absatz deshalb
im nativen Dokumenttext und fragt dort einen UTF-16-Bereich ab. Breite, Höhe
und Bildschirmposition werden mit dem gezeichneten Quellplan abgeglichen;
die Absatzrolle bleibt erhalten. Primärquellen sind die lokal festgelegten
`accesskit_consumer/src/text.rs` (`supports_text_ranges`) und
`accesskit_macos0.27.1/src/node.rs` (`frame_for_range`).

Drei abwechselnde lokale Metal-Paare auf einer unveränderten18-Dateien-Kopie
vergleichen die App-/UI-Quellen0.9.50 mit der neuen Fassung. Die Mittelwerte der
Wechsel-Layoutzeit sind2,48→2,87ms,7,38→2,70ms,2,53→2,57ms; die jeweiligen
Frame-p95 sind27,38→28,31ms,99,70→26,11ms,32,37→32,53ms. Der Ausreißer
und verbleibende Raster-/Presentspitzen erlauben keine allgemeine
Beschleunigungs- oder60-fps-Aussage. Originale:
`build/reader-geometry-verification/paired-profiles.json` und dessen Einzelprotokolle.
Die Referenz-App-/UI-Dateien stammen unverändert aus dem Quellcommit
`bd61ba99535d650045b6b0fb031e636e132889a7`; unveränderte Kern-/Textbibliotheken
und derselbe Profilprüfer werden geteilt.

Ausgeführte Gesamt-/Sanitizer-/Paketprüfungen stehen in [STATUS](STATUS.md).
Weitere Titel-/Markierungsgeometrie, neue Zielplattformen und tatsächliche
IME-/Screenreader-/Geräte-/Langzeitabnahmen bleiben Release-Arbeit.
