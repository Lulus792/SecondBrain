# Erstellen in der kompakten Werkzeugleiste

Stand:11.Oktober2026; Recherche vor dem Entwurf, Umsetzung0.9.64.

Bei schmalen Fenstern oder großer Schrift zeigt die Werkzeugleiste für „Neue
Notiz“ und „Neues Projekt“ bisher zweimal dasselbe Plus. Hinweise und native
Namen unterscheiden sie, die sichtbaren Symbole selbst jedoch nicht. Umgesetzt:
Dokument mit Plus für eine neue Notiz, Ordner mit Plus für ein neues Projekt.
Dasselbe Motiv begleitet auch die weiteren Zugänge zum Anlegen eines Projekts.
Das einzelne Plus bleibt für den Zoom erhalten.

Apple [Icons](https://developer.apple.com/design/human-interface-guidelines/icons),
Best practices, und [Toolbars](https://developer.apple.com/design/human-interface-guidelines/toolbars),
Actions, am11.Oktober2026 über titelgeprüfte offizielle DocC-Daten gelesen:
einfache erkennbare Metaphern, gemeinsame Größe/Detailtiefe/Strichstärke,
verständliche Bedeutung der Toolbar-Aktionen und alternative Textbeschreibungen.
HTML lieferte nur die JavaScript-Hülle. Projektübertragung: Unsere beiden
Erstellaktionen bekommen unterschiedliche Gegenstandsmotive mit demselben Plus.
Das ist eine eigene Gestaltung, keine Übernahme der SF-Symbol-Grafiken.

Die neuen C-Strecken und SVGs werden gemeinsam aus `tools/export_icons.py`
erzeugt.24er-Raster,1,7er-Strich und bestehende Farbanbindung bleiben die Basis.
Vor Veröffentlichung sind die tatsächliche Darstellung in heller/dunkler/
kontrastreicher Ansicht und100/150/200Prozent Schrift sowie die richtigen
Maus-/Tastaturaktionen, Namen und Abbruchwege nachzuprüfen. Produktänderungen
sind Teil des vollständigen Umsetzungsauftrags; der Reviewbericht liegt außerhalb
unter UI_reviewer. Weitere Release-Abnahmen bleiben eigenständig.

## Geprüfter Stand

Vorher-/Nachheraufnahmen aus der echten C-App wurden angesehen. Neun Kombinationen
100/150/200Prozent Schrift mit dunkler/heller/dunkler Kontrastdarstellung öffnen
per SDL-Maus jeweils das richtige Formular; Escape bricht ab. Ganze Tastatur-
abläufe, native Mac-Anbindung, Interaktion/Layout und Ebenenprüfung bestehen im
gezielten Neuner-Lauf (141,20s). Keine menschliche Desktop-/Screenreader-Abnahme.
29 vorhandene SVGs bleiben bytegleich,33 generierte C-/SVG-Dateien reproduzieren
beim zweiten Export exakt. Projektvorlagen/Python-Generator sind nicht geändert.

Bei der Bildkontrolle wurde außerdem SBUI-083 korrigiert: ausgewählte/fokussierte
Sternlabels reservieren ihre Flächen zuerst. Normale konkurrierende Labels werden
mit der bestehenden Kollisionsregel ganz ausgelassen; ein deckender Kontrast-
hintergrund übermalt keine Teile ihres bereits gezeichneten Worts. Die reproduzierte
200Prozent-Aufnahme ist nachgeprüft. Große reale Graphen und weitere Prioritäts-/
Gerätefälle bleiben gesonderte Abnahme.

Originale: `build/creation-icons-verification/`. Review mit SBUI-082/083 außerhalb:
`/Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-11_22-01-59/UI_REVIEW.md`.
Paket und neue Zielplattformabnahme folgen. Frühere82/82 von0.9.63 sind kein
neu ausgeführter64-Gesamtlauf; für diesen begrenzten Darstellungsschritt wurden
die neun passenden vorhandenen Prüfungen ausgeführt.

0.9.64/bef4816d3e40 als sauberes Paket geprüft und installiert:98 Dateien
paketgleich,18 Gedächtnisdateien unverändert, aktuelle Aufnahme angesehen.
126 Desktop-,167 Tastatur-,75 Sicherungsbedingungen, Neustarts und CLI-/
Runtime-/Lizenzprüfung bestehen. Neue CI38171616070 zu bef4816d3e40 zuletzt
Windows/Linux in Arbeit, Mac queued; keine neue pauschale Plattformabnahme.
