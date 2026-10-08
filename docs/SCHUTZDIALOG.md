# Ungespeicherte Änderungen erhalten

Stand: 8. Oktober 2026, Umsetzung ab 0.9.39.

Vor einem Dokument-/Projektwechsel oder dem Beenden schützt die App einen
ungespeicherten Entwurf. Der Dialog verwendet dieselbe Kopfzeile und dieselben
Innenabstände wie die übrigen Karten. Schließen sitzt rechts im Kopf.

Der Hinweis und eine mögliche Fehlermeldung stehen in einer eigenen Scrollfläche.
Speichern, Verwerfen, gegebenenfalls Sichern als neue Notiz und Weiter bearbeiten
bleiben darunter fest erreichbar. Die Höhe berücksichtigt den tatsächlichen
Textumbruch und die verfügbare Fensterhöhe. Lange Meldungen können mit Mausrad,
Bild auf/ab oder der Scrollschiene gelesen werden; der Hintergrund scrollt dabei
nicht. Die vorhandene begrenzte elastische Bewegung gilt auch hier.

Der anfängliche Fokus liegt auf Speichern und weiter. Tab führt durch die
vorhandenen Aktionen einschließlich Schließen. Escape, Schließen und Weiter
bearbeiten brechen ausschließlich den ausstehenden Wechsel ab. Sie erhalten
Entwurf und gespeicherte Originalfassung. Enter aktiviert das fokussierte Ziel;
es verwirft nicht automatisch Änderungen. Ein neu geöffneter Dialog beginnt
wieder oben. Der vorhandene Konfliktweg bleibt unverändert: Eine erkannte extern
geänderte Datei wird nicht durch die eigene Fassung überschrieben.

## Grundlage und Grenzen

Apple empfiehlt kurze, handlungsbezogene Hinweise, klar benannte Aktionen,
einen sicheren Abbruch und die Anpassung an größere Texte. Defaultaktionen
stehen bei gestapelten Buttons oben, Abbrechen üblicherweise unten.
[Alerts](https://developer.apple.com/design/human-interface-guidelines/alerts),
[Layout](https://developer.apple.com/design/human-interface-guidelines/layout),
offizielle DocC-Inhalte am 8. Oktober 2026 gelesen.

Die feste Kopf-/Aktionsfläche und der scrollbare Meldungskörper sind eine eigene
Übertragung auf diese C-App. Der Konfliktzustand hat vier Aktionen und ist keine
native Apple-Alert-Komponente. Der Testumfang und die tatsächlich nachgewiesenen
Plattformen stehen in [STATUS.md](STATUS.md). Reale Trackpads, native Screenreader
und menschliche Bedienabnahmen werden dadurch nicht ersetzt.
