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
