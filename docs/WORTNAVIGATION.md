# Wörter bewegen und auswählen

Stand: 8. Oktober 2026, Umsetzung ab 0.9.40.

Die Texteingabe verwendet eigene C-Wortgrenzen nach Unicode 18.0.0,
UAX #29 Revision 49. Buchstaben, kombinierte Zeichen, Zahlen mit passenden
Trennzeichen, Apostrophe, Identifier und Emoji erhalten zusammenhängende
Segmente. Die UI überspringt beim Wortwechsel Segmente ohne Buchstaben,
Zahlen, Identifier-Verbinder oder Emoji. Doppelklick wählt das getroffene
Segment einschließlich seiner kombinierenden Zeichen, ohne folgende Leerzeichen.
Auch die bisherige rechte Wortauswahl verwendet diese Geometrie.

| Aktion | macOS | Windows/Linux |
| --- | --- | --- |
| Zum vorherigen Wortanfang | Option+Links | Strg+Links |
| Nach rechts bewegen | Option+Rechts: Wortende | Strg+Rechts: nächster Wortanfang |
| Auswahl erweitern | zusätzlich Umschalt | zusätzlich Umschalt |
| Zeilenanfang/-ende | Command+Links/Rechts | vorhandene Zeilentasten |

Ein nicht erweiterter Wortbefehl klappt eine bestehende Auswahl zunächst an
entsprechender Seite zusammen. Navigation verändert keinen Text. Die Auswahl
bleibt an ganzen Graphemen; Löschen und Rückgängig erhalten die originalen
UTF-8-Bytes. Wortgrenzen gehören zum vorhandenen Text-/Schrift-/Zeilencache,
einschließlich seiner Speichergrenze und Invalidierung. Bei Key-up werden auch
nach vorherigem Loslassen des Modifikators alle zugehörigen Pfeilzustände beendet.
Ereignisse fremder Fenster werden nicht als Wortbefehl übernommen.

Ein Buttonereignis liefert seine aktuelle Mausposition mit Bruchteilen an den
UI-Adapter. Der Klick braucht kein zuvor verarbeitetes Motion-Ereignis. Beim
Doppelklick entscheidet die tatsächliche Glyphen-/Graphemfläche über das Wort;
der nächstgelegene Cursor am rechten Rand eines Buchstabens darf nicht
versehentlich das folgende Leerzeichen auswählen.

## Grundlage und Prüfung

[Unicode UAX #29](https://www.unicode.org/reports/tr29/tr29-49.html), Abschnitt 4,
am 8. Oktober 2026 gelesen. Default-Grenzen benötigen sprachabhängige Ergänzungen
für echte Wörterbuchsegmentierung, etwa in Thai oder Chinesisch. Diese App
implementiert keine solche Analyse. Die vorhandenen [Grapheme](GRAPHEME.md)
bleiben der Schutz gegen Teilzeichen. Die Wortbefehle verwenden logische
Quellreihenfolge; gemischte Schreibrichtungen sind keine vollständige Zusage
nativer, sprachabhängiger Wortbewegung.

Die Mac-Kürzel folgen [Apples Textbearbeitungskürzeln](https://support.apple.com/en-us/102650).
Die übrigen Systeme behalten Strg als Wortmodifikator und den nächsten Wortanfang
als rechtes Ziel. Originaldaten, Hashes und Unicode License V3 stehen in
[third_party/unicode](../third_party/unicode/README.md). Eine neue Bibliothek ist
nicht erforderlich; Python aktualisiert ausschließlich die Entwicklerdaten.

Die Kernprüfung liest alle 1.944 unveränderten offiziellen WordBreakTest-Fälle.
Dazu kommen ungültige/leere Eingaben, eine lange Zeichenkombination und 200.000
aufeinanderfolgende Regional Indicators. Die UI-Prüfung verwendet echte
SDL-Tasten- und Buttonereignisse, drei Schriftgrößen, Apostrophe, Zahltrennzeichen,
Emoji, kombinierte Akzente, mehrzeilige Texte, Auswahl, Löschung und Undo sowie
Modifier-Key-up und fremde Fenster. Die genaue Abnahme steht in [STATUS](STATUS.md).
Native Screenreader-/Textrechtecke, sprachabhängige Bedienung und reale Geräte
bleiben weitere Release-Arbeiten.
