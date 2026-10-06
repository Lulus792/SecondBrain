# SecondBrain Offene Fragen

Stand: 6. Oktober 2026.

## Abnahme der ersten Version

Geklärt: Die [Abnahme zu 287ef48](https://github.com/Lulus792/SecondBrain/actions/runs/37445131476)
besteht auf allen drei Systemen einschließlich Debug/Release, eigener Projektinstanz,
Konfliktkopie, Archivierung, Beenden und entpackter Pakete.
Für den festgelegten ersten lokalen Arbeitsablauf ist keine Abnahmefrage offen.

## Aktuelle Designauswahl

Geklärt: Lumen liefert die gewünschte Farbpalette. Karten und Bedienelemente
sollen stärker wie Apples Liquid Glass gestaltet werden. Die verfeinerte
Vorschau ist erstellt und geprüft.

Offen für die Integration: der Materialweg auf Windows, macOS und Linux sowie
Kamerabedienung und Graphumfang. Apples native macOS-Komponente ist ab macOS 26
dokumentiert; die Vorschau selbst ist eine optische Nachbildung.
Beleg: [Designrecherche](../../docs/UI_GALAXIE.md).

## Weitere mögliche Erweiterungen

Diese Fragen blockieren den ersten lokalen Arbeitsablauf nicht.
Sie sind noch keine Implementierungsaufträge oder zugesagten Funktionen.

- Welche automatische KI-Pflege wäre hilfreich? Aktuell liest und pflegt eine
  ausdrücklich beauftragte KI die Dateien und prüft Originalquellen.
- Soll Wissen über mehrere Projekte gemeinsam durchsucht oder synchronisiert werden?
- Wie werden Vorlagen migriert, ohne individuelle Inhalte zu ersetzen?
- Welcher UI-Zugang kann native Screenreader vollständig unterstützen?
- Welche weiteren Schriftsysteme, Markdown-Elemente und Suchnormalisierungen
  werden in der tatsächlichen Nutzung gebraucht?
- Werden signierte Pakete, Installer und dauerhafte Release-Downloads benötigt?

Anforderungen und Abgrenzung: [Projektauftrag](PROJECT.md).
