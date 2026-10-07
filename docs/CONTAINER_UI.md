# Listen und Zitate in der Leseansicht

Stand: 7. Oktober 2026. Vor der App-Anbindung recherchierter Entwurf, in 0.9.17 umgesetzt.
Aktuelle Abnahme und Grenzen stehen in [STATUS](STATUS.md).

Apples [Layout](https://developer.apple.com/design/human-interface-guidelines/layout),
[Lists and tables](https://developer.apple.com/design/human-interface-guidelines/lists-and-tables)
und [Accessibility](https://developer.apple.com/design/human-interface-guidelines/accessibility)
wurden erneut über die offiziellen DocC-Daten gelesen. Apple empfiehlt erkennbare
Ausrichtung, Einrückung für Hierarchie, lesbare Inhalte und zugängliche Beschreibungen.
Die folgende Darstellung ist eine eigene Übertragung auf schreibgeschütztes Markdown,
keine von Apple vorgegebene Markdown-Komponente.

Die bestehende Glaskarte bleibt die Lesefläche. Listen rücken ihre Kinder ein;
Punktmarker werden als kleine eigene Kreise gezeichnet, geordnete Einträge erhalten
ihre tatsächliche fortlaufende Nummer. Fortsetzungen bleiben am Text ausgerichtet.
Zitate erhalten eine seitliche Linie und Einrückung. Verschachtelung bleibt im
Dokumentbaum vollständig, der sichtbare Einzug wird bei schmaler Breite begrenzt,
damit für lesbaren Text Platz bleibt. Lockere Listen/Absätze erhalten mehr Abstand.

Native Listen, Einträge und Zitate gruppieren die tatsächlichen Blattkinder.
Marker und Inhalt bleiben getrennt, Aktionen bleiben im jeweiligen Blatt erreichbar.
Tab erzeugt keine zusätzlichen Stopps auf reinem Text; vorhandene Linkbuttons,
Reader-Fokus und Alt+Bild auf/ab bleiben die Bedienwege. Quellpositionen für IDs
und Abschnittssprünge stammen aus der unveränderten Originaldatei.

Titel, Graph, Tabellen und Reader verwenden den gemeinsamen Dokumentbaum mit
seiner Referenzumgebung. Code und HTML bleiben literal. Fehler erhalten Dateien
und Entwürfe; bei Parserfehlern zeigt der Reader Originaltext, der Graph behält
seinen letzten gültigen Stand. Erfolg, kleine Fenster, 200-Prozent-Schrift,
Kontrast, native Struktur, Tastatur und Originalbytes sind getrennt zu prüfen.
Menschliche Screenreader- und tatsächliche Plattformabnahme bleiben erforderlich.

Native Containerrechtecke vereinigen sichtbare Blatt-/Aktionsbereiche. Rein
logische Kinder bekommen keine erfundenen sichtbaren Rechtecke. Lange Nummern
reservieren ihre gemessene Breite; Marker stehen in der nativen Lesereihenfolge
vor dem Text. Abschnitts- und Containeranfragen verwenden denselben Scrollweg.
