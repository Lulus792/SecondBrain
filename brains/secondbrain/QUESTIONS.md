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

Geklärt in Version 0.2: Die C-App verwendet einen eigenen Materialrenderer auf
allen drei Plattformen. Kamerabedienung und Graphumfang sind implementiert und
abgenommen. Apples native Komponente ist weiterhin kein Bestandteil der App.
Beleg: [Designrecherche](../../docs/UI_GALAXIE.md) und [Umsetzung](../../docs/UI_TASTATUR.md).

## Vollständiger Release

Der Nutzer hat einen begründeten Mindestversionsvorschlag beauftragt. Stand
12. Oktober 2026: macOS 14.0 (Intel/ARM64), Windows 11 25H2 x64 und Ubuntu
24.04 LTS x86_64 sind vorgeschlagene Abnahmeziele. Die tatsächliche Abnahme und
verbindliche Zusage stehen noch aus; [Begründung und Quellen](../../docs/PLATTFORM_VORSCHLAG.md).
Die eigene Lizenz ist
auf Nutzerentscheidung MIT. Die Arbeiten der Release-Liste sind beauftragt;
1.0 wird erst auf ausdrückliche Freigabe gesetzt. Signierte Pakete benötigen
noch Apple-Developer-Konto beziehungsweise Windows-Signaturzertifikat. Die [Release-Liste](../../docs/RELEASE.md)
hält die erforderlichen Arbeiten und ihre Abnahme fest. Die abschließende
sprachliche Bereinigung bleibt ein Schritt vor dem fertigen Endprodukt.

## Weitere mögliche Erweiterungen

Diese Fragen blockieren den ersten lokalen Arbeitsablauf nicht.
Sie sind noch keine Implementierungsaufträge oder zugesagten Funktionen.

- Welche automatische KI-Pflege wäre hilfreich? Aktuell liest und pflegt eine
  ausdrücklich beauftragte KI die Dateien und prüft Originalquellen.
- Soll Wissen über mehrere Projekte gemeinsam durchsucht oder synchronisiert werden?
- Wie werden Vorlagen migriert, ohne individuelle Inhalte zu ersetzen?
- Native Screenreader sind Teil des beauftragten Release-Umfangs; welche
  tatsächlichen assistiven Prüfungen sind auf allen Systemen ausführbar?
- Welche weiteren Schriftsysteme, Markdown-Elemente und Suchnormalisierungen
  werden in der tatsächlichen Nutzung gebraucht?
- Werden signierte Pakete, Installer und dauerhafte Release-Downloads benötigt?

Anforderungen und Abgrenzung: [Projektauftrag](PROJECT.md).

## Vertrauliche Sicherheitsmeldungen

Am 12. Oktober entschieden: **GitHub Private Vulnerability Reporting**.
Am selben Tag aktiviert und geprüft: GitHub bestätigt die Einstellung; die
öffentliche Advisories-Seite bietet **Report a vulnerability** an. Keine
Testmeldung abgeschickt. Dieser Punkt ist abgeschlossen; [Richtlinie](../../SECURITY.md).
Öffentliche Fehlerformulare bleiben ein anderer Weg. Vertrauliche Inhalte werden
nicht in Issues veröffentlicht.
