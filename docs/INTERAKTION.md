# Reaktionszeit, Übergänge und Inhaltsflächen

Stand: 7. Oktober 2026, 0.9.18. Erneute Politur nach konkretem Nutzerfeedback
und Screenshot. Recherche vor Umsetzung: [UI_POLITUR](UI_POLITUR.md).

Eine angenommene Notizwahl wirkt fachlich sofort. Die Kamera startet aus ihrer
aktuellen Position und bewegt sich mit einer frühen sichtbaren Reaktion zum
neuen Stern. Die Fahrt dauert regulär 420 ms, die kurze Überblendung des bereits
sichtbaren Karteninhalts 180 ms. Beide sind unterbrechbar. Direkte Aktionen auf
der Karte beenden die alte Überblendung; neue Pfeilwahlen setzen die Bewegung
von ihrem aktuellen Stand fort. Reduzierte Bewegung übernimmt den Zielzustand
sofort. Ungespeicherte Änderungen behalten den bestehenden Schutzdialog.

Die Hauptschleife zählt Renderarbeit und Wartezeit gemeinsam zum Bildbudget.
Vorgemerkte Navigation und noch nicht begonnene Fahrt lösen den Leerlauf bereits
auf. Das alte Kartenbild wird vor Present beim Wechsel gespeichert; normale
Bilder werden direkt gerendert. Fehlende Snapshot-Ressourcen verhindern keine
Navigation. Diese Wege sind keine Zusage einer festen Bildrate für jedes Gerät.

Scrollen behält die Position als Fließkommazahl. Der Rest gegenüber dem
Ganzpixel-Offset wird bei der Darstellung verwendet und bei der Größenmessung
wieder ausgeglichen. Dokument-/Quellpositionen bleiben unabhängig davon. Der
Schieber benutzt die tatsächliche Position; Ziehen folgt unmittelbar dem Zeiger
und verwirft alte Wheel-Anteile. Die begrenzte Rückmeldung an den Kanten bleibt.

## Dialoge, Hinweise und Suche

Bereichs- und Aktionsmenüs bemessen ihre Breite anhand der Beschriftungen.
Dialogkörper werden anhand ihres tatsächlichen Layouts gemessen; Kopf,
Abschluss und Innenränder kommen hinzu. Die Fenstergrenze begrenzt die Fläche,
längere Körper bleiben scrollbar. Erste Größenkorrekturen bekommen einen
zeitnahen nächsten Frame. Schriftgröße, Zustand und verfügbare Breite gehören
zu den Messbedingungen. Ein Projektwechsel stellt die normale Ansicht wieder
her, damit die Dokumentliste erreichbar bleibt.

Zeilen und Buttons haben skalierende linke Ränder. Hinweise erscheinen nach
350 ms ruhigem Hover als eigene nichtinteraktive Overlay-Zeichnung. Sie nehmen
keinen Fokus und blockieren keine Klicks. Ihre blickdichte Fläche, Innenränder
und Kürzelplakette werden getrennt gezeichnet; die Position bleibt im Fenster.
Passende Kürzel werden nach Plattform angezeigt. Modale Ansichten unterdrücken
Hinweise der dahinterliegenden Bedienelemente.

Suchsymbol, Texteingabe, Placeholder und Löschung teilen eine Fläche und
Ausrichtung. Die Löschaktion hat eine kleine eigene Form ohne zusätzliche
Glaskarte. Ein leeres Feld benötigt keine Löschaktion; bei Inhalt bleibt sie
auch per Tastatur erreichbar.

## Nachweise und Grenzen

Die gezielte ASan/UBSan-Prüfung deckt zusätzlich den Readerbeginn und das
Wachsen des UI-Zeichenpuffers ab. Die Interaktionsprüfung umfasst frühe Bewegung, tatsächlich ausgelesenen
Snapshot-Inhalt, zwölf schnelle Wechsel, direkte Aktionen, Bewegungsreduktion,
fraktionales Scrollen, Zeigerziehen, Dialoggeometrie, zwölf Formtypen bei
100/200 Prozent, 780×640, Suchzustände, Hinweis-Pixel und Klicks unter Hinweisen.
Tatsächlich ausgeführte Gesamt-, native, Sanitizer- und Paketprüfungen stehen
in [STATUS](STATUS.md). Bilder/Providerabfragen ersetzen keine menschliche
VoiceOver/NVDA/Orca- oder Geräte-/Langzeitabnahme.

Der Materialvergleich prüft 48 konkrete Raster gegen den vorherigen eigenen
Renderer pixelgenau. Die lokale Metal-Zeitmessung betrifft macOS 14.6.1 auf
Intel i7-8559U. Neue Messungen mit Bildtakt sind methodisch anders als der
ursprüngliche Lauf ohne Takt; sie belegen keine pauschale FPS-Steigerung.
Renderzeitspitzen bleiben Gegenstand weiterer Leistungsabnahmen.

Grundlagen: Apple [Motion](https://developer.apple.com/design/human-interface-guidelines/motion),
[Menus](https://developer.apple.com/design/human-interface-guidelines/menus) und
[Search fields](https://developer.apple.com/design/human-interface-guidelines/search-fields),
vor Umsetzung über offizielle DocC-Daten gelesen. Zahlen, Overlay-Technik und
konkrete Layoutregeln sind eigene Übertragung. 1.0 bleibt bis zur Nutzerfreigabe
gesperrt; weitere [Release-Arbeiten](RELEASE.md) bleiben bestehen.
