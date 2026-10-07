# Windows: Textsuche im nativen Textbereich

Stand: 7. Oktober 2026, Entwicklungsschritt 0.9.13; native Abnahme noch offen.
Die [erste 0.9.12-Abnahme](https://github.com/Lulus792/SecondBrain/actions/runs/37624868119)
scheitert unter Windows Debug/Release am ersten Stilfall. HRESULT 0, aber kein
zurückgegebener Bereich. Die tatsächlich gelesene Quelle des festgelegten
Adapters AccessKit Windows 0.35.1 enthält für `FindText` nur eine nicht
implementierte Methode. Linux Debug/Release besteht dieselben Stilabfragen.
Dies ist ein konkreter Windows-Adapterbefund; der Test wird nicht entfernt.

## Vertrag und Umsetzung

Vor der Änderung gelesen: Microsofts
[ITextRangeProvider::FindText](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itextrangeprovider-findtext)
liefert einen passenden Teilbereich, den ersten oder letzten Treffer und optional
Suche ohne Beachtung der Großschreibung. Kein Treffer bedeutet Erfolg mit leerem
Ausgabepointer. Die Suche bleibt im angefragten Bereich.
[FindStringOrdinal](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-findstringordinal)
liefert UTF-16-Indizes, vorwärts/rückwärts und ordinalen Vergleich. Dieser Vergleich
ist keine sprachliche oder kanonisch normalisierte Suche.

Die eigene kleine UI-Ergänzung benutzt diese Betriebssystemfunktion und die
vorhandenen AccessKit-Positionen. UTF-16-Treffer werden zum ursprünglichen
Dokumentbereich zurückgeführt. Leere oder fehlerhafte UTF-16-Suchwerte und
Längen außerhalb der Windows-API-Grenze werden zurückgewiesen. Fehlende Treffer
liefern einen leeren Pointer; Systemfehler bleiben Fehler. Der Adapter verändert
keinen Text und erweitert einen begrenzten Suchbereich nicht.

Die fachliche Projekt-/Notizsuche bleibt eigener C-Code. Die Rust-Ergänzung liegt
innerhalb der erlaubten externen UI-Bibliothek; eigene Anwendung und fachlicher
Kern bleiben C17. Die Ergänzung steht unter der eigenen MIT-Lizenz. Originale
AccessKit-Lizenzen und Autoren bleiben zugeordnet.

## Festgelegter Quellbuild

C-Bindings 0.23.1, Commit 8b6ed37c20ed4c59390e253407983333053662ba:
Archiv-SHA-256 `f15581c841eed0f2f6cec6a6f9b7fd4ca9d34a654546efa22ae29351efa06568`.
Windows-Adapter 0.35.1: Crate-SHA-256
`ce63f35d6bdcf59f26b76b3379063f738e6412cef46999cc772d46aa3de35adb`.
Original src/text.rs: `7f2493b559f0b04884e201d897981a3cce47b2c571863d37ffddb1bb695e5c55`;
geprüfte Änderung: `0fd8cba2e66e5edcaed3fa8d35572e530b7a816a1a716703e96867440709bae1`.
Der Vorbereitungsschritt erlaubt ausschließlich Original oder exakt diese
geänderte Quelle; unbekannte Quellen werden abgewiesen.

Cargo baut mit dem festen Lockfile; andere Paketversionen und Abhängigkeiten
werden nicht verändert. Die Windows-DLL bleibt eine getrennte UI-Bibliothek mit
ihrem eigenen Allokator. CMake kopiert und packt die tatsächlich gebaute DLL.
Windows-UI-Builds benötigen Cargo/Rust ab 1.87; fertige Pakete und reine Kernbuilds
benötigen keine Rust-Toolchain. Für MinGW x64 benötigt Cargo zusätzlich den
Windows-GNU-Zielstandard und dessen passenden Linker. Native MinGW-Abnahme ist offen.

## Prüfung

Die originale Vorbereitung, Wiederholung ohne Änderung, Zurückweisung einer
veränderten Fremdquelle und unveränderten übrigen Lockfile-Pakete sind lokal
geprüft. `cargo metadata --locked` löst den Windows-Graphen auf. Vier lokale
Mac-Nachprüfungen bestehen (19,25 s), darunter 380 native, 85 Text- und 111
Editorassertions. Das ist keine Windows-Kompilierung oder Ausführung.

Die tatsächlichen Windows-Stilabfragen bleiben erhalten. Zusätzliche UIA-Fälle
prüfen ersten/letzten Treffer, Großschreibung, fehlenden Treffer, UTF-16-Indizes
nach einem Emoji, einen Treffer am Anfang und einen begrenzten Teilbereich.
Ausführung und entpacktes Windows-Paket werden erst anhand der neuen nativen CI
abgenommen. Die neue Arbeit ist bis dahin noch keine bestätigte Windows-Behebung.
Menschliche Screenreader-Bedienung und präzise Textgeometrie bleiben getrennte Abnahmen.
