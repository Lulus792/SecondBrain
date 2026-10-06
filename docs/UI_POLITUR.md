# Bedienung, Bewegung und Layout vor 1.0

Stand: 6. Oktober 2026. Der aktuelle Nutzerauftrag umfasst die Behebung der
Archiv-Navigation, Scrollgrenzen, Menü-Pfeiltasten und sichtbaren Layoutfehler,
eine große Leseansicht, integrierte Suchfeld-Löschung und eigene Icons. Zusätzlich
sind die offenen 1.0-Aufgaben beauftragt. Die Versionsnummer 1.0 bleibt bis zur
expliziten Freigabe des Nutzers gesperrt.

## Erneut eingesehene Apple-Grundlagen

Originale einschließlich der öffentlichen Dokumentationsdaten wurden vor dieser
Überarbeitung gelesen:

- [Scroll views](https://developer.apple.com/design/human-interface-guidelines/scroll-views):
  gewohnte Gesten und Tasten, elastische Grenzrückmeldung, keine vertikal
  verschachtelten Scrollbereiche; angeheftete Bedienelemente vom Inhalt trennen.
- [Menus](https://developer.apple.com/design/human-interface-guidelines/menus):
  konkrete Aktionen, kurze Bezeichnungen, konsistentes Verhalten und Tastaturwege.
- [Buttons](https://developer.apple.com/design/human-interface-guidelines/buttons) und
  [Icons](https://developer.apple.com/design/human-interface-guidelines/icons):
  erkennbare Funktionen, konsistente Konturen und brauchbare Ziele.
- [Motion](https://developer.apple.com/design/human-interface-guidelines/motion):
  nachvollziehbare Bewegung, Unterbrechbarkeit und reduzierte Bewegung.
- [Layout](https://developer.apple.com/design/human-interface-guidelines/layout) und
  [Accessibility](https://developer.apple.com/design/human-interface-guidelines/accessibility):
  Anpassung an Fenster und Schriftgröße, sichtbarer Fokus und Textalternativen.

## Daraus abgeleitete Umsetzung

Dokumentkarten haben feste Kopf- und Fußleisten und einen einzigen scrollenden
Inhalt. Kleine Karten öffnen sich auf Wunsch als große Leseansicht. Schließen
sitzt rechts im Kopfbereich; Dialogaktionen werden von den Formularfeldern getrennt.
Listen reservieren Platz für Filter und Seitenwechsel. Die Bereichsauswahl bleibt
auch außerhalb der Liste erreichbar. Ein Archivdokument schaltet den Filter nicht
ungefragt um.

Die Scrollposition endet an einer gemessenen Inhaltsgrenze. Elastische Bewegung
ist lediglich eine begrenzte optische Verschiebung; sie verändert die Inhaltsgröße
und die gespeicherte Scrollposition nicht. Dadurch darf wiederholtes Scrollen am
Ende kein Hochspringen oder wechselnde Grenzwerte auslösen. Neue Tastaturfokusse
werden gezielt sichtbar gemacht. Menüpfeile wechseln Aktionen und überspringen
den Schließen-Knopf; Textcursor und Lesebereich behalten ihre üblichen Pfeilwege.

Die Raumbewegung verschiebt die Kamera zwischen tatsächlichen Weltpositionen der
Notizen. Eine kurze Fahrt in der Tiefe und räumliche Hintergrundpartikel erzeugen
Parallaxe. Neue Ziele beginnen an der aktuellen Kameraposition. Kartenwechsel
ändern den Bildausschnitt weich. Reduzierte Bewegung überspringt die Übergänge.

Eigene Vektor-Icons liegen in assets/icons. Die Originalkonturen werden in C
gezeichnet und durch tools/export_icons.py als SVG exportiert. Sie verwenden weder
Schriftzeichen als Ersatz noch externe Icon-Schriften. Die Herkunft ist der eigene
MIT-lizenzierte Code. Tooltip und Textbezeichnung begleiten die Funktionen.

## Erforderliche Abnahme

Archiv und Rückkehr, wiederholtes Scrollen an beiden Grenzen, Richtungswechsel,
Menü-Pfeile, große Leseansicht, integriertes Suchfeld, unterbrechbare Raumfahrt,
Fokus und alle Leisten bei kleinen Fenstern und 100/150/200 Prozent Schrift prüfen.
Screenshots werden aus der tatsächlichen App aufgenommen. Native Plattform- und
Paketnachweise werden erst nach den betreffenden Läufen eingetragen.

## Zusätzliche Befunde der Implementierung

Die frühere gemeinsame Scrollsteuerung von Hintergrundnotiz und Projektkontext
wurde getrennt. Dokumente und Dialoginhalte verwenden eigene Scrollgrenzen und
schmale, ziehbare Scrollbalken; die UI-Bibliothek begrenzt dieselbe Position nicht
zusätzlich. Die Helligkeit von Sternen unter dunklem Glas ist begrenzt, damit
Beschriftungen auch während einer Fahrt lesbar bleiben. Tooltips haben einen
lesbaren Hintergrund. Kleine Ansichten reduzieren zuerst Zusatzinformationen und
verwenden bei großen Schriften beschriftete Icon-Funktionen mit Tooltips.

Die 1.0-Arbeiten bleiben vollständig beauftragt. Die bislang geprüfte UI ist ein
Entwicklungsschritt; Sicherung, dauerhafte Einstellungen, native Ordnerauswahl,
Screenreader-Anbindung, weitere Daten-/Leistungsprüfungen und Distribution sind
weitere Schritte desselben Ziels. Die Versionsnummer bleibt bis zur Freigabe unter 1.0.
