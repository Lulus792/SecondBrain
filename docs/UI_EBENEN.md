# Fokus und Beschriftungen vor transparenten Karten

Stand: 11. Oktober 2026, Umsetzung in 0.9.47.

Die Sternkarte hat zwei Rollen: dekorativer räumlicher Hintergrund und
funktionale Annotationen mit Namen, Auswahl und Tastaturfokus. Eine transparente
Karte kann eine dahinter gezeichnete Linie weiterhin durchlassen. Deshalb
schützt Zeichenreihenfolge allein ihre Textfläche nicht.

Die Annotationen werden auf die frei sichtbare Kartenfläche begrenzt.
Lesekarte, Dokumentliste und Kameraleiste verwenden ihre tatsächlichen Grenzen;
die Kamera-Geometrie wird beim Zeichnen und Ausschneiden gemeinsam berechnet.
Zwei Vordergrundflächen ergeben höchstens16 getrennte Rechtecke. Ganzzahlige,
konservative Schnittkanten erhalten den Schutz auch an geglätteten Rändern.
Der Fokusrahmen liegt innerhalb des Clips, damit seine volle Strichbreite bleibt.
Sterne, Verbindungen und Glasmaterial bleiben Teil des dekorativen Hintergrunds.

Vollständig verdeckte Karten und die Hintergrundkarte eines Editors sind keine
Tabziele. F6 deckt die Sternkarte weiterhin auf. Deckt das Suchende die gerade
erreichte Karte wieder ab, setzt sich der Fokus im geöffneten Dokument fort.
Bei Dialogrückkehr wird eine vorher aktive Karte tatsächlich aufgedeckt;
eine ausdrücklich angeforderte neue Editieraufgabe behält Vorrang.
Eine Notizauswahl öffnet die Karte erst nach akzeptiertem Modellwechsel,
damit die alte Editorfläche keinen laufenden Entwurfsentscheid vorwegnimmt.

Apple beschreibt in [Focus and selection](https://developer.apple.com/design/human-interface-guidelines/focus-and-selection)
plattformspezifische Fokusdarstellung und sinnvolle Rückkehr bei verschwindenden
Zielen. [Materials](https://developer.apple.com/design/human-interface-guidelines/materials)
behandelt die Trennung funktionaler Ebenen und lesbaren Inhalts. Best practices
bzw. Materialrollen/macOS wurden am11.Oktober über offizielle DocC-Daten erneut
gelesen. Rechtecksubtraktion, Tabverhalten und Rasterprüfung sind eigene
Übertragungen auf die bestehende C-App, keine Apple-API oder vorgeschriebenen Maße.

Der neue Rasterprüfer vergleicht Vordergrundstreifen mit/ohne Kartenfokus und
weist den gleichzeitig sichtbaren Fokus im freien Bereich nach. Er erfasst
hell/dunkel, hohen Kontrast, zwei Fenstergrößen,100/200-Prozent-Schrift,
Liste/Kameraleiste, verdeckten Editor, Suchrückkehr und F6. Der bisherige Code
verletzt bereits die erste dunkle Kartenprüfung. Ereignistests ergänzen
Tastatur-, Such-, Schutzdialog- und Interaktionsabläufe. Ausgeführte Ergebnisse
stehen in [STATUS](STATUS.md); reale assistive und weitere Plattformabnahmen
bleiben getrennt.
