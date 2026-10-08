# Unicode-Daten für eigene C-Zeichengrenzen

Festgelegt: Unicode 18.0.0, UAX #29 Revision 49; eingesehen am 7. Oktober 2026.
Originale: https://www.unicode.org/Public/18.0.0/ucd/,
https://www.unicode.org/reports/tr29/tr29-49.html.

GraphemeBreakProperty, Indic_Conjunct_Break aus DerivedCoreProperties und
Extended_Pictographic aus emoji-data sind Eigenschaftsdaten. GraphemeBreakTest
ist der unveränderte offizielle Testbestand. Es wird keine externe Unicode-
Bibliothek eingebunden. Die Segmentierung ist eigener C-Code; die Daten werden
mit tools/make_grapheme_data.py in src/grapheme_data.inc überführt. Python wird
nur bei einer bewussten Aktualisierung der Entwicklerdaten benötigt, weder
beim normalen C-Build noch zur Anwendung. Die Runtime liest keine dieser Dateien.

Die Daten und die abgeleiteten Tabellen stehen unter Unicode License V3.
Der Originaltext steht in ../licenses/Unicode.txt und wird mitgeliefert; die
Datenhinweise bleiben erhalten. Änderungen der festgelegten Eingaben werden
vom Generator abgewiesen, bis ihre Version und erwarteten Hashes bewusst
aktualisiert wurden. Alle Unicode-Testfälle müssen anschließend neu bestehen.

SHA-256 der Originale:

- `GraphemeBreakProperty.txt`: `0839dcb79e4ac639ecd538b1abf7c9d22e3f9dd265b7e182d33627aa4d75b45a`
- `DerivedCoreProperties.txt`: `09c928886a178fcafd93c29e4bd59073a058e5a100b716d425cb563ab50f68c9`
- `emoji-data.txt`: `80d00f8e616a0ef27fd6b8de3b758c06383b5d917e2977709578e68baf733bf1`
- `GraphemeBreakTest.txt`: `b0cf047ee94485bbdc846de2b902f5f8a815f6b674f9d04223cddadd91c9df31`


## Zeichengruppen für Hervorhebungen

DerivedGeneralCategory.txt aus Unicode 18.0.0, Originalquelle
https://www.unicode.org/Public/18.0.0/ucd/extracted/DerivedGeneralCategory.txt,
SHA-256 d6b151d2d40ee9b1876d26f417980f45ffae47b6055ccf7203cb31f07a030f94.
Die Datei bleibt unverändert. tools/make_inline_unicode.py erzeugt daraus die
C-Bereiche für P-/S-Kategorien nach Hashprüfung. Runtime/Build benötigen den
Generator nicht. Daten und abgeleitete Tabelle fallen unter die bereits
mitgelieferte Unicode License V3; eigener Leser bleibt MIT-lizenzierter C-Code.


## Referenznamen

CaseFolding.txt aus Unicode 18.0.0 bleibt unverändert unter 18.0.0/.
Original: https://www.unicode.org/Public/18.0.0/ucd/CaseFolding.txt.
SHA-256: a004797658a457bec4dc11683e39f69249ea3b595b752dbea6721c4c9f587b0d.
tools/make_casefold_data.py erzeugt nach Hashprüfung die vollständigen
Standard-C-/F-Mappings in src/casefold_data.inc; S-/T-Mappings sind nicht gewählt.
Tabelle SHA-256: cb4376660d9c92d5d55648692586459c31a929504299ece3c8a2bc003da4290a.
Die Originaldaten und die abgeleitete Tabelle verwenden die bestehende Unicode
License V3; eigener C-Vergleich/Definitionsleser bleibt MIT. Kein Runtime-
Datenladen und kein Generatorbedarf beim normalen C-Build.

## Bidi-Absatzlayout (UI)

Die unveränderten Unicode-18-Originale BidiTest, BidiCharacterTest,
BidiMirroring, BidiBrackets, DerivedBidiClass, PropList, PropertyValueAliases
und Scripts ergänzen die vorhandenen Daten. Herkunft: derselbe UCD-18-Ordner;
DerivedBidiClass stammt aus `extracted/`. Die Hashes sämtlicher benötigter
Eingaben stehen in `../ui/bidi18/manifest.json`. Eigenschaftsdaten und abgeleitete
Tabellen verwenden die bestehende Unicode License V3. Anders als die eigene
Graphemsegmentierung verwendet der UI-Absatzalgorithmus eine externe C-UI-
Bibliothek; der fachliche Kern bleibt ohne externe Bibliotheken.
[Vertrag und tatsächliche Nachweise](../../docs/BIDI.md).

## Wortgrenzen und Tastatureingabe (0.9.40)

WordBreakProperty.txt und WordBreakTest.txt stammen unverändert aus
https://www.unicode.org/Public/18.0.0/ucd/auxiliary/. UAX #29 Revision 49
wurde am 8. Oktober 2026 gelesen. Eigener C-Code in src/word.c implementiert
die Default-Wortgrenzen; keine externe Unicode-Bibliothek.
tools/make_word_data.py erzeugt die Wort-, Emoji- und Buchstaben-/Zahlbereiche
nach Hashprüfung. Auch der originale Testbestand ist festgelegt. Python wird
nur für bewusste Entwickleraktualisierung und Prüfungen verwendet.

Die UI unterscheidet navigierbare Segmente mit Buchstaben, Zahlen,
Identifier-Verbindern oder Emoji von sonstigen Segmenten. Das ist eine eigene
Navigation auf den normativen Grenzen; es ist keine Wörterbuchanalyse.
Die vorhandene Unicode License V3 wird weiterhin mitgeliefert.

Zusätzliche SHA-256 der Originale:

- `WordBreakProperty.txt`: `8dbfa17063e11084201f33c3e76d485d3b9166930c71db8e39ed1c9234171aec`
- `WordBreakTest.txt`: `3dd70c071781276067c680d87303f60434adce7b067bd063194af374edad86a5`
