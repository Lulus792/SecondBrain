# Unicode-18-Daten für den C-UI-Absatzalgorithmus

Basis: SheenBidi 3.0.0, Apache License 2.0. Unveränderter Originaltext:
[SheenBidi-LICENSE](SheenBidi-LICENSE). Originalquellen werden beim UI-Prüfbuild
als SHA-256-festgelegtes Archiv geladen; der C-Kern verlinkt sie nicht.

`BidiTypeLookup.c` und `PairingLookup.c` sind aus Unicode-18-Daten mit dem
originalen Entwicklergenerator erzeugt. Lokale Änderung am Generator:
Paarabstände und erzeugte Differenzen verwenden 32 Bit statt 16 Bit, damit
U+221D ↔ U+1DB10 korrekt bleibt. Die Angaben zur Tabellengröße berücksichtigen
das größere Element. Der eigene Entwicklerstarter
`tools/make_bidi_data.py` prüft zuerst Originalarchiv und betroffene
Generatorquellen, wendet diese Änderung in einem temporären Ordner an und
prüft alle Eingaben gegen `manifest.json`. `--check` vergleicht neu erzeugte
Bytes mit den festgelegten Hashes und Repositorydateien.

Unicode-Originale und daraus abgeleitete Eigenschaftsdaten verwenden Unicode
License V3, `third_party/licenses/Unicode.txt`. Generierter Lookupcode und der
übernommene Generator unterliegen der mitgelieferten Apache-2.0-Lizenz.
Die übrigen Script-/Kategorie-Lookups der Bibliothek werden nicht aktualisiert
und nicht über die eigene C-API angeboten. Diese Grundlage ist noch keine
Runtime-Abhängigkeit; vor der produktiven Anbindung gehören ihre Hinweise
auch in Anwendungspakete und Lizenzinventur. [Vertrag und Nachweise](../../../docs/BIDI.md).
