# Fokus in modalen Karten

Stand: 11. Oktober 2026, lokale Korrektur 0.9.62.

Wenn Kontext oder Lizenztext über einem Dokument geöffnet sind, gehört der
Tastaturfokus zur vorderen Aufgabe. Beide Lesebereiche verwenden intern dieselbe
Kennung. Bis 0.9.61 konnte deshalb auch das abgeschirmte Dokument einen blauen
Rahmen zeichnen, obwohl seine nativen Ziele bereits gesperrt waren.

Die gemeinsame Ringzeichnung berücksichtigt jetzt zusätzlich die aktive Gruppe.
Bei Formularen oder Schutzdialogen zeichnen nur Ziele der Modalgruppe einen
Fokusrahmen. Nach Escape erhält das Dokument seinen normalen Rahmen zurück.

## Nachweis

`tests/test_modal_focus.c` liest tatsächliche Renderer-Pixel: Der vordere Rahmen
muss sichtbar werden, ein Hintergrundstreifen unverändert bleiben. Nach einem
SDL-Escape-Ereignis müssen Formular, Fokus und Dokumentrahmen zurückkehren.
Die Fixture setzt die geteilte Kennung ausdrücklich, um die Zeichnung zu
isolieren; dies ist keine menschliche Desktopbedienung.

310 Bedingungen bestehen über Kontext und Lizenztext, 100/200 Prozent Schrift
sowie dunkle/helle, reguläre/solide und kontrastreiche Darstellung. Die gleiche
Regression scheitert mit dem bisherigen Produktcode am Hintergrundstreifen.
Fünf gezielte Tests bestehen zusätzlich (71,10 Sekunden). Vorher-/Nachherbilder
wurden angesehen. Originale: `build/modal-focus-verification/`.

Geprüfte Umgebung: macOS 14.6.1, Intel, Retina. Neue Windows-/Linux-/ARM64-,
menschliche Tastatur- und Screenreaderabnahmen bleiben getrennte Nachweise.
Schriftgrößenprüfung ist kein Ersatz für unterschiedliche Displayskalierungen.

## Gestaltung

Apple beschreibt Fokus als Orientierung für das Interaktionsziel und Modalität
als begrenzte getrennte Aufgabe: [Focus and selection](https://developer.apple.com/design/human-interface-guidelines/focus-and-selection)
und [Modality](https://developer.apple.com/design/human-interface-guidelines/modality),
Best practices, am 11. Oktober 2026 erneut gelesen. Die Gruppenkontrolle im eigenen
C-Renderer ist unsere Umsetzung dieser Prinzipien, keine Apple-API-Vorgabe.
SBUI-081 und Quellenkopien stehen im externen UI-Review-Bericht vom selben Tag,
21:12:33. Dieser Schritt schließt die vollständige Release-Liste nicht ab.
