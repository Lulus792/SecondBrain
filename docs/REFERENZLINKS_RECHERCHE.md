# Nächster Parserabschnitt: Referenzlinks

Stand: 7. Oktober 2026. Geplante Implementierung, noch kein Produktnachweis.
Originalquellen: [CommonMark 0.31.2, Definitionsblöcke](https://spec.commonmark.org/0.31.2/#link-reference-definitions)
und [Referenzlinks](https://spec.commonmark.org/0.31.2/#links), am selben Tag gelesen.

Eine Definition gehört zur ganzen Datei und darf vor oder nach ihrer Verwendung
stehen. Die erste passende Definition gewinnt. Sie wird nicht als Absatz gezeigt.
Definitionen unterliegen Blockregeln: höchstens drei führende Leerzeichen,
kein Unterbrechen eines laufenden Absatzes, keine Erkennung innerhalb von Code.
Ziel und optionaler Titel dürfen unter festgelegten Regeln auf Folgezeilen stehen.
Ein fehlerhafter Titel auf derselben Zeile verwirft die Definition; ein ungültiger
optionaler Folgetitel kann als eigener Text bleiben. Definitionen in Listen und
Zitaten benötigen den noch fehlenden vollständigen Containerleser.

Vollständige `[Text][Name]`, verkürzte `[Name][]` und einfache `[Name]`-Formen
sowie entsprechende Bilder verwenden dieselbe Definitionsliste. Namen haben
höchstens 999 Unicode-Zeichen, verbieten unmaskierte innere eckige Klammern und
benötigen Inhalt außer Leerraum. Der Vergleich faltet Unicode-Groß-/Kleinschreibung,
entfernt Rand-Leerraum und fasst interne Leerzeichen, Tabulatoren und Zeilenenden
zusammen. ASCII-Vergleich allein genügt nicht. Maskierungen und Entities im
Namen dürfen nicht beiläufig denselben Vertrag wie dekodierte Linkziele bekommen.
Originalfälle hierzu werden vor Umsetzung gezielt ausgewählt und unverändert
gegen echte C-Ausgabe geprüft.

Die bestehende Inline-API leiht nur den Text eines Absatzes. Für Referenzen muss
sie zusätzlich eine unverändernde dokumentweite Umgebung erhalten. Die gemeinsame
Blockerkennung muss Definitionsbereiche liefern und aus sichtbaren Blöcken
entfernen. Leseansicht, Tabellen, Titel, Graph und Testprobe brauchen dieselbe
Umgebung; eine reine UI-Ersetzung wäre unvollständig. Definitionsnamen benötigen
festgelegte Unicode-Casefold-Daten mit Lizenz und eigenem C-Vergleich, geprüfte
Speicher-/Arbeitsgrenzen sowie eine geordnete Suche. Fehler müssen die vorhandenen
Quellen und den letzten gültigen Graph erhalten. Der nächste Schritt ist diese
C-Umgebung samt Daten- und Blockvertrag, danach ihre Integration in alle Leser.
