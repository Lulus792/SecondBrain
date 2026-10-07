# Eigener Dokumentbaum in C

Stand: 7. Oktober 2026. Implementierter Kernbaustein. Ab 0.9.17 verwenden Titel,
Leseansicht, Tabellen und Sternkarte den gemeinsamen Baum. Die installierte App
bleibt bis zur neuen Paketabnahme beim zuvor geprüften 0.9.16-Paket.
[Darstellung und weitere Abnahme](CONTAINER_UI.md).

`src/document.c/h` baut einen eigenen Baum für Wurzel, Zitat, Liste und Eintrag
mit Absätzen, Überschriften, Code, Trennlinien, Literal-HTML und Tabellen. Eltern,
Kinder und Geschwister behalten Quellreihenfolge und ursprüngliche Bytebereiche.
Jedes Textblatt besitzt eine getrennte [Quellprojektion](CONTAINER_PLAN.md):
Containerpräfixe verschwinden in dieser Ansicht, die Datei bleibt unverändert.
Physische Tabspalten und teilweise verbrauchte Tabulatoren bleiben korrekt.
Auch leere Blätter haben einen passenden ursprünglichen Randpunkt.

Offene Container werden pro Zeile fortgesetzt oder geschlossen. Fehlende Präfixe
sind nur als erlaubte Absatzfortsetzung zulässig. Listenbeginn, Marker-/Trennzeichen-
wechsel, Einrückung, leere Einträge, kompakte/lockere Listen und verschachtelte
Container gehören zur Erkennung. Blattregeln für Überschriften, Zäune und
Trennungen verwenden den vorhandenen gemeinsamen Zeilenprüfer. Code behält
innere Leerzeilen/Einrückung, abschließende Leerzeilen des eingerückten Codes
werden mitsamt ihrer aktiven Positionszuordnung gekürzt. Tabellen erkennen die
bestehenden eigenen GFM-Regeln innerhalb des Blattkontexts.

Referenzdefinitionen stehen am Anfang eines Absatzblatts. Definitionen werden
vor Setext-Umwandlung geprüft; ATX-Überschriften erzeugen sie nicht. Die Umgebung
sammelt Definitionen aus dem ganzen Baum, einschließlich Listen und Zitaten.
Erste Definition und Unicode-Faltung bleiben die vorhandenen Verträge. Jede
Definition leiht ihre tatsächliche Blattprojektion; eine unpassend geglättete
Gesamtdatei wird nicht erzeugt. Die Referenzumgebung muss vor dem Dokument
freigegeben werden. `sb_document_content` liefert den sichtbaren Blattbereich,
Code/Literal-HTML behalten ihren Literalvertrag. HTML wird nicht ausgeführt.

## Grenzen und Fehler

Quellen benötigen gültiges UTF-8 ohne NUL und bleiben bei höchstens 16 MiB.
Der derzeitige Baum unterstützt höchstens 65.536 Knoten einschließlich Wurzel
und 64 offene Container. Die Parserarbeit wird durch die Quellenlänge begrenzt;
Projektions-/Referenzgrenzen gelten zusätzlich. Überschreitungen werden als
Fehler gemeldet, nicht als erfolgreiches halbes Dokument. Die Bereitschafts-
markierung wird erst nach vollständigem Abschluss gesetzt; aus einem fehlerhaften
Baum lässt sich keine Referenzumgebung ableiten. Auch nach Fehlern muss der
Aufrufer den frischen Speicher freigeben. Die Originalquelle bleibt während
der Baum-/Referenznutzung geliehen und erhalten.

Diese Bausteingrenzen sind geprüft; die abschließende gemeinsame App-/Daten-
abnahme ist noch offen. Der spätere Renderer muss Fehler im Literalmodus zeigen
und die C-Graphfunktion ihren letzten gültigen Stand behalten. Eine stabile
vorherige Desktop-Inventur bleibt in [RELEASE](RELEASE.md) offen.

## Nachweise

307 unveränderte [CommonMark-Originalfälle](../tests/data/commonmark-0.31.2/README.md)
vergleichen den tatsächlichen C-Baum mit einer unabhängigen HTML-Struktur:
Reihenfolge, Eltern/Kinder, Listentyp/Anfang, kompakt/locker, Blatttyp, Ebene und
Klartext. Zwei HTML-Interaktionsfälle folgen dem ausdrücklich bestehenden
Literalvertrag; Kommentare werden als Literalblöcke verglichen. Der Prüfer
behauptet keine Inline-Stil-/Zielabnahme. Dafür bestehen eigene Tests.

Eigene Fälle prüfen Referenzen aus verschiedenen Projektionen, erste Definition,
Unicode-Ziele, Code-Ausschluss, ATX-/Setext-Abgrenzung, Quellbereiche, geschlossene
Knoten, Speicher-/Knoten-/Tiefengrenzen, eine exakte 16-MiB-Quelle und 4.000
begrenzte Eingaben mit erhaltenen Originalbytes. Gezielte ASan/UBSan umfasst den
Dokument-/Projektions-/Markdown-/Referenz-/Inline-/sb-Code; übrige Plattformteile
sind nicht vollständig instrumentiert, Leakprüfung ist auf diesem macOS nicht
verfügbar und deaktiviert. Ausgeführte Ergebnisse stehen in [STATUS](STATUS.md).

Originale wurden vor Umsetzung gelesen: [CommonMark 0.31.2, Container](https://spec.commonmark.org/0.31.2/#container-blocks)
und [Parsing strategy](https://spec.commonmark.org/0.31.2/#appendix-a-parsing-strategy).
Die offizielle Referenzimplementierung wurde für Randfälle studiert; der Kern
enthält eigene C-Implementierung, keine zusätzliche externe Fachbibliothek.

## Folgende Arbeit

Titel, Leseansicht, Tabellen und Graph verwenden jetzt dieselbe Baum-/Referenzumgebung.
Die App ergänzt Listeneinrückung, Nummern, Zitatlinien und native Containerrollen.
Tabellen-/Zeilenkennungen und Abschnittssprünge verwenden ursprüngliche
Quellpositionen. Aktuelle Tastatur-/Raster-/Provider- und Plattformnachweise
stehen in [STATUS](STATUS.md); die vollständige assistive Abnahme bleibt offen.
Auch übrige Markdown-Regeln und [Release-Aufgaben](RELEASE.md) bleiben bestehen;
307 ausgewählte Fälle sind keine vollständige CommonMark-/GFM-Abnahme.
