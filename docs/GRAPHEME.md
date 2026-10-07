# Zeichen als Einheit bearbeiten

Grundlagen eingesehen am 7. Oktober 2026 vor der UI-Änderung:
[UAX #29 Revision 49](https://www.unicode.org/reports/tr29/tr29-49.html),
[Unicode 18.0.0](https://www.unicode.org/versions/Unicode18.0.0/).
Die erweiterte Graphemvariante umfasst kombinierte Akzente, Hangul, Flaggen,
Emoji-Verbindungen und indische Verbindungszeichen. Unicode 18 ändert die
GB9c-Regel; die Implementierung übernimmt diese aktuelle Variante.

Eigener Entwurf: Alle Nuklear-Eingaben verwenden die gleiche C-Segmentierung.
Links/rechts, Auswahl und Löschen behandeln vollständige Zeichen. Cursor und
native Auswahl bleiben intern als Unicode-Skalarpositionen gezählt, werden aber
auf Graphemgrenzen begrenzt. Auswahl innerhalb eines Zeichens wird nach außen
erweitert. Texteingaben und Auswahlersetzungen werden vor der Mutation auf
Kapazität geprüft. Ein Texteingabeereignis bildet einen Undo-Vorgang; eine
Überschreiboperation ersetzt vollständige vorhandene Grapheme. Die UTF-8-
Originalbytes werden dabei weder normalisiert noch beim bloßen Lesen verändert.

Die konkrete Segmentierung und ihre Datentabellen sind eigener C-Code und
[festgelegte Unicode-Daten](../third_party/unicode/README.md). Leere Texte und
ungültiges UTF-8 haben geprüfte Grenzen. Der Segmentierer kann U+0000 aus dem
Normtestbestand verarbeiten; die bestehenden Datei-/Eingabevalidatoren weisen
NUL unverändert ab. Der Iterator arbeitet ohne Speicherallokation und Rekursion.
Positionsabfragen scannen den vorhandenen Text; große reale Texte benötigen
weiterhin eine gesonderte Leistungsabnahme.

Normative Grenzen, UI-Bedienung und native APIs werden getrennt geprüft. Eine
bestandene Zeichengrenzenprüfung beweist keine Emoji-Schriftabdeckung, gemischte
Schreibrichtungen, IME-Kandidatenposition oder native Zeichenrechtecke.
Diese Aufgaben und menschliche VoiceOver/NVDA/Orca-Abnahmen bleiben offen.


## Lokale Nachweise während der Umsetzung

19 Debug-Kernprüfungen bestehen (35,13 s). Der eigene Iterator und Positions-
abfragen bestehen 18.764 Assertions über alle 853 offiziellen Unicode-18-Fälle,
leere/ungültige Eingaben und eine 100.001 Byte lange Kombination; ASan/UBSan
besteht ebenfalls. Vier gezielte UI-/native-/Lizenz-/Normprüfungen bestehen
(52,42 s): 101 Editor-, 273 native und 56 Lizenz-Assertions. Die native macOS-
Auswahl einer einzelnen Akzentposition wird auf den vollständigen Text erweitert.
Die drei weiteren Zielarchitekturen werden nach Push nativ geprüft.


Der vollständige lokale Release-Lauf besteht mit 33 Tests (270,67 s).
ASan/UBSan prüft zusätzlich die eigenen UI-, Graphem- und Testobjekte und besteht
mit 101 Editor-Assertions; die eingebundenen UI-Bibliotheken sind dabei nicht
vollständig instrumentiert. Ohne die neuen Eingabehaken scheitert derselbe
Editorfall an edit.cursor==0: die alte Einzelzeichen-Navigation lässt den Cursor
innerhalb des Akzents stehen. Die Datenerzeugung reproduziert bytegleich die
festgelegte C-Tabelle. Das neue entpackte Intel-Paket wird separat geprüft.


## Nach der Rasterprüfung

Die isolierte Editoransicht zeigte abgeschnittene Hangul-/Verbindungs- und
Emoji-Zeilen. Ursache: addierte Einzelzeichenbreiten stimmten nicht mit dem
geformten Textlauf überein; der Widget-Clamp kürzte deshalb dessen Ausgabe.
Zeilen und Auswahlpositionen werden mit der gemeinsamen Schriftbibliothek als
Textlauf gemessen. Der Editor zeichnet vollständige UTF-8-Läufe und begrenzt
sie geometrisch. Neue Kommandoprüfungen verlangen vollständige Hangul-,
Devanagari-, Emoji-, Ligatur- und Akzentzeilen. Emoji-Fontabdeckung und komplexe
Maus-/Bidi-Geometrie bleiben davon getrennte Aufgaben.

Ein Paketdurchlauf scheiterte außerdem, weil sich die Systemzwischenablage
zwischen gesetztem Testpfad und dem Paste-Ereignis änderte. Die Sicherungs-
Pfadfixtures verwenden nun echte SDL-Texteingabeereignisse. Die eigentliche
App-Zwischenablage und die Prüfung des Kopierens einer Fehlermeldung bleiben
bestehen. Der fehlerhafte Paketdurchlauf ist kein bestandener Nachweis; das
Paket wird mit der letzten Umsetzung erneut erzeugt und entpackt geprüft.


Die abschließende Nachprüfung der geformten Zeilen besteht mit sechs Tests
(100,34 s), darunter 111 Editor-, 273 native und 75 Sicherungsassertions.
Die eigene UI-Instrumentierung unter ASan/UBSan besteht abschließend mit 111
Assertions. Ein weiterer Paketlauf findet dieselbe externe Zwischenablage-
abhängigkeit bei einem Lizenz-Kopiertest. Testdaten werden jetzt über SDL-
Texteingaben gesetzt; der tatsächliche Kopierinhalt wird unmittelbar nach der
Aktion aus SDL gelesen und vor zusätzlichen Zeichenframes festgehalten.
Die normale Anwendung und ihr Kopierweg werden dadurch nicht ersetzt.


Die letzten Testeingabe-Anpassungen bestehen im Desktopablauf mit 126 Assertions
und nach UTF-8-sicherer Abschnittszufuhr im Tastaturablauf mit 142 Assertions
(53,65 s). Kopierprüfungen lesen echte SDL-Zwischenablagewerte unmittelbar nach
der Aktion. Lange Fixtures werden in vollständigen UTF-8-Abschnitten innerhalb
des vorhandenen Ereignislimits zugeführt; zuvor fehlte bei einer Einzelzufuhr
der Rest eines langen Linkfixtures. Die normalen App-Eingaben und Kopieraktionen
bleiben unverändert. Das abschließende Paket wird erneut erzeugt.
