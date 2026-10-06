# Recherche und Designrichtungen für eine Wissensgalaxie

Stand: 6. Oktober 2026. Status: Recherche und vergleichbare Designentwürfe.
Der Nutzer möchte zuerst auswählen. Die räumliche Ansicht ist noch keine
Funktion der C-Desktop-App.

## Auftrag und bestätigte Gestaltung

Der Nutzer beschreibt einen Sternen- oder Galaxienraum, in dem man sich bewegen
und Wissen erkunden kann. Die Grundidee gefällt ihm; die konkrete Gestaltung
der gefundenen Referenz trifft seinen Geschmack noch nicht.

Die Antworten auf die Designfragen legen fest:

- Dunkle und elegante Grundstimmung mit dezenten Farben.
- Notizen als kleine leuchtende Sterne mit klaren Beschriftungen.
- Wenige schwebende Bedienelemente. Eine Notiz öffnet sich bei Auswahl.
- Zusätzlich ein vergleichbarer Entwurf mit glasartigen Karten über der Galaxie.

Die bisherigen Anforderungen an C, UI-Abhängigkeiten, drei Zielplattformen und
die eigene Wissensanwendung bleiben maßgeblich. Es wurde noch kein Entwurf
für die Umsetzung ausgewählt.

## Eingesehene Referenzen

| Referenz | Beobachtetes beziehungsweise dokumentiertes Konzept | Ableitung für SecondBrain |
| --- | --- | --- |
| [ReverySky](https://moonskorch.github.io/reverysky/) und [Originalrepository](https://github.com/moonskorch/ReverySky-Plugin) | Ein dreidimensionaler Raum verbundener Notizen. Verschieben, Zoomen, Drehen, Auswahl einer Notiz und Betrachtung ihrer Nachbarschaft sind dokumentiert. Die Sternraum-Darstellung auf der Originalseite wurde tatsächlich betrachtet. | Räumliche Navigation, Auswahl eines Sterns, fokussierte Nachbarschaft und Zusammenhang zwischen Karte und Notiz. |
| [GalaxyBrain](https://github.com/trungnguyenarts/GalaxyBrain) | Ein dreidimensionales Wissensnetz aus Markdown-Notizen. | Lesbare Projektdateien können die Grundlage einer räumlichen Ansicht bleiben. |
| [The Knowledge Cosmos](https://www.theknowledgecosmos.com/) | Wissenschaftliche Publikationen werden als Sternenraum mit Themen und Konstellationen dargestellt. | Überblick über Themen, Übergang vom größeren Zusammenhang zum einzelnen Inhalt. |
| [Cosma Handbuch](https://cosma.arthurperret.fr/user-manual.html) | Interaktives Netz von Wissenskarten mit Text, Suche und Fokusmodus. | Die räumliche Karte benötigt einen verlässlichen Zugang zum lesbaren Inhalt. |

Die Referenzen dienen der Gestaltung. Es wurden keine Plugins installiert,
keine fremden Laufzeitkomponenten in die App eingebunden und keine realen
Projektdaten an die Referenzdienste übertragen.

## Apple-Gestaltungsgrundlagen

Die bestehende [UI-Recherche](UI_RECHERCHE.md) liefert Hierarchie, Typografie,
Navigation und Kontrast. Ergänzend wurden Apples öffentliche Dokumentationsdaten
zu [Materialien](https://developer.apple.com/design/human-interface-guidelines/materials)
und [Bewegung](https://developer.apple.com/design/human-interface-guidelines/motion)
eingesehen.

Apple trennt funktionale Bedienelemente von der Inhaltsebene. Liquid Glass ist
für Navigation und Bedienelemente gedacht; für Inhalte beschreibt Apple
Standardmaterialien. Für die Notizkarten wird deshalb ein eigener zurückhaltender
Blur-Eindruck vorgeschlagen, ohne ihn als native Liquid-Glass-Komponente auszugeben.
Schrift, Kontrast und die Hierarchie müssen trotz sichtbarem Hintergrund erhalten bleiben.

Bewegung soll der Handlung dienen, kurz und nachvollziehbar sein. Daraus folgen
direkte Kamera-Reaktion auf Eingaben, abbrechbare Fokusbewegung und eine reduzierte
Bewegungsvariante. Eine ständig automatisch rotierende Galaxie gehört nicht zum
vorgestellten Arbeitsablauf.

## Drei Entwürfe zur Auswahl

Die Vorschauen zeigen ausdrücklich Beispieldaten. Positionen und Beziehungen
sind illustrative Entwurfsdaten, keine neue Analyse des Physim-Repositories.

### Lumen

Ein freier, dunkler Sternenraum mit wenigen Elementen. Projekte bilden erkennbare
Gruppen; Notizen sind kleine Lichtpunkte. Eine Auswahl öffnet eine kompakte
schwebende Lesekarte. Ein geschlossener Textbereich gibt den Raum wieder frei.

Diese Richtung legt den Schwerpunkt auf Erkunden und eine ruhige Oberfläche.

### Glas

Die Galaxie bleibt als Hintergrund sichtbar. Eine glasartige Hauptkarte zeigt
die ausgewählte Notiz; kleinere Karten zeigen ihre Verbindungen. Blur, ein
dezenter Rand und begrenzte Transparenz trennen Text und Sterne.

Diese Richtung legt den Schwerpunkt auf räumliche Tiefe und sichtbare
Zusammenhänge. Anzahl und Größe der Karten müssen begrenzt bleiben.

### Fokus

Die ausgewählte Notiz und ihre unmittelbaren Verbindungen treten hervor.
Weiter entfernter Inhalt bleibt zurückhaltend im Raum. Eine größere schwebende
Lesekarte liegt unter dem zentralen Sternbild.

Diese Richtung legt den Schwerpunkt auf vertieftes Lesen mit räumlichem Kontext.

## Geprüfte Vorschau

Der [Quellstand der drei Vorschauen](design/galaxie-entwuerfe.html) ist als
Designstudie versioniert. Er ist ein Vorschaufragment für den Vergleich der
Oberflächen und kein Laufzeitbestandteil der C-Anwendung.

Die drei Entwürfe wurden im Browser betrachtet. Ein tatsächlicher Klick auf
einen Stern in Lumen öffnet die Beispielnotiz. In Glas wurden Kamera-Drehung,
Auswahl einer verknüpften Karte und das Schließen der Karten geprüft. Titel,
Text und Verknüpfungen wechseln mit der Auswahl. Die Vorschau meldet dabei
keine JavaScript-Fehler. Diese Prüfung ist ein Nachweis der Designvorschau,
kein neuer Plattformnachweis für eine räumliche C-Oberfläche.

## Nächste Auswahl

Offen ist, welcher Entwurf oder welche Kombination weiter ausgearbeitet wird.
Danach werden Kamerabedienung, Lesekarte, Beschriftung, Graphumfang und die
Umsetzung innerhalb der eigenen C-Anwendung konkretisiert. Sichtbare Beziehungen
sollen in der späteren App aus nachvollziehbaren Projektverweisen entstehen.
