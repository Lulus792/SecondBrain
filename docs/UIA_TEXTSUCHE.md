# Windows: Textsuche im nativen Textbereich

Stand: 7. Oktober 2026, Entwicklungsschritt 0.9.13; Windows-/Linux-Abnahme bestanden, neue Mac-CI noch offen.
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
geprüfte Änderung: `453fcaaa4faf52c87cc8f15fc50d4710540e35bc181bd6ac3daf28613ed7874a`.
Der Vorbereitungsschritt erlaubt Original, exakt bekannte bisherige Patchfassungen und diese endgültige
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
abgenommen. Die neue Arbeit wurde anschließend tatsächlich unter Windows geprüft;
den abgeschlossenen Umfang nennt der folgende Nachweis.
Menschliche Screenreader-Bedienung und präzise Textgeometrie bleiben getrennte Abnahmen.


Der erste Quellbuild-Lauf 37628165496 stoppt unter Windows Debug/Release schon
im Konfigurationsschritt. Die vorhandenen Annotationen enthalten nur Exit 1;
Ursache ist noch nicht belegt. Öffentliche REST-Logabfrage ist nicht verfügbar,
der veröffentlichte HTML-Logverweis liefert keinen zugänglichen Detailblock.
Ein Konfigurationswrapper erfasst jetzt beide Ausgabeströme, erhält den
Fehlerstatus und veröffentlicht die Ursache als Annotation sowie checked-configure.log.
Eine absichtlich fehlschlagende lokale Konfiguration bestätigt Status, Log und
Prozent-Escaping; YAML ist mit Psych gelesen. Neuer nativer Diagnoselauf folgt.
Dies ist Diagnoseverbesserung und keine behauptete Windows-Behebung.

Das lokale Intel-Paket 0.9.13 zu 2242b5d besteht bereits mit Desktop 126,
Tastatur 142, Sicherung 75, Neustart und CLI; Log windows-findtext-package-check.log.
Dist bleibt vorerst bei geprüftem 0.9.12. Paketprüfung ist kein Windows-Nachweis.


Der Diagnoselauf 37629012326 stoppt ebenfalls beim Windows-Konfigurationseinstieg;
keine der vorgesehenen Konfigurationsdateien wird als Artefakt gefunden. Die
Ursache ist damit weiterhin unbekannt. Der Einstieg verwendet nun ausdrücklich
Bash, vollständig zitierte absolute Quell-/Buildpfade und ein zusätzliches
Shell-Einstiegslog. Ein separater Berichtsschritt veröffentlicht dessen Inhalt,
auch wenn der innere Wrapper noch nicht erreicht wird. Prozent-/Zeilen-Escaping
ist mit der echten lokalen Fehlerdatei geprüft; Workflow-YAML ist gelesen.
Eine vollständige lokale CMake-Fixture bindet die Quellbuild-Integration ein und
konfiguriert erfolgreich; sie kompiliert keine Windows-Bibliothek.
Die Quellpatch-Prüfung akzeptiert auch eine CRLF-Fassung des eigenen Snippets;
daraus wurde kein reproduzierter Fehler abgeleitet. Neuer nativer Diagnoselauf folgt.


Der ausführliche Windows-Nachweis 37630397578 nennt nun die Ursache:
CMake schreibt das geänderte text.rs mit CRLF. Sein tatsächlicher Hash
`4c9836aa3845b12cdca68c8f7bbad0b1f6ae88d7f4ba10911a2e5881f86f798f`
ist bytegenau der CRLF-Fassung des vorgesehenen LF-Ergebnisses. Eine CRLF-Fassung
des eingelesenen eigenen Snippets war hingegen lokal kein Fehler; der relevante
Unterschied entsteht beim Windows-Dateischreiben.

Der Vorbereitungsschritt verwendet jetzt file(CONFIGURE) mit NEWLINE_STYLE UNIX.
Originalquelle, bereits vorbereitete LF-Fassung und exakt die hinterlassene
Windows-CRLF-Fassung ergeben denselben festgelegten LF-Hash. Unbekannte Quellen
werden weiterhin abgewiesen. Alle vier lokalen Quellenfälle bestehen; Logs in
build/windows-findtext-lineend-proof. Kein temporärer Konfigurationseingang bleibt
als Buildabhängigkeit zurück. Neue tatsächliche Windows-Konfiguration/Kompilierung
und native Ausführung folgen; noch keine Windows-Abnahme behaupten.


Die native Windows-Konfiguration zu e6c30d0 besteht; der Quellbuild meldet nun
zwei tatsächliche Typfehler: windows-strings 0.5.1 stellt BSTR als Deref<[u16]>
bereit, nicht mit as_wide; windows-result 0.4.1 verwendet Error::from_thread,
nicht Error::from_win32. Beide Aufrufe sind anhand der originalen, gehashten
Lockfile-Quellen korrigiert. Der neue Ergebnis-Hash ist
`453fcaaa4faf52c87cc8f15fc50d4710540e35bc181bd6ac3daf28613ed7874a`.
Original, beide exakt bekannten bisherigen Fassungen und neue LF-Fassung
werden reproduzierbar auf diese Version gebracht; unbekannte Quellen bleiben
abgewiesen. Vier Quellenfälle und Abweisung bestehen lokal. Windows-Kompilierung
und native Clientausführung werden im neuen Lauf weiter geprüft.


## Tatsächliche Windows-/Linux-Abnahme zu 9c38604

Der [Lauf 37631991014](https://github.com/Lulus792/SecondBrain/actions/runs/37631991014)
besteht unter Windows und Linux in Debug und Release, einschließlich der nativen
Stilabfragen. Windows kompiliert die neue UI-DLL aus den festgelegten Quellen;
die neun FindText-Fälle bestehen mit ersten/letzten Treffern, Großschreibung,
fehlenden Treffern, Emoji-Indizes, Anfang und begrenzten Suchbereichen.
Beide tatsächlich entpackten Release-Pakete bestehen ihre Abläufe. Insgesamt
bestehen alle zwölf Windows-/Linux-Jobs;
neue Mac-Jobs sind beim dokumentierten Zwischenstand noch in der Warteschlange.

Damit sind FindText und die vorher fehlgeschlagenen Windows-Stilabfragen im
automatisierten Umfang nachgeprüft. Keine menschliche NVDA-/Orca-/VoiceOver-
Abnahme daraus ableiten. MinGW, genaue Textgeometrie und weitere Release-
Aufgaben bleiben offen. Die strenge Quellenprüfung bleibt erhalten; der
endgültige Patchhash lautet 453fcaaa4faf52c87cc8f15fc50d4710540e35bc181bd6ac3daf28613ed7874a.
