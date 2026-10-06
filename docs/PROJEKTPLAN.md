# Projektplan für die SecondBrain Anwendung

Stand: 6. Oktober 2026. SecondBrain soll eine eigene Anwendung in C werden,
mit der der Nutzer und die KI projektbezogenes Wissen verwenden können.
Die Anwendung übernimmt das Anlegen, Anzeigen, Bearbeiten und Verwalten der
Projektgedächtnisse. Die folgenden Anforderungen sind verbindlich festgehalten;
die Umsetzung und der UI-Entwurf sind noch ausstehend.

## Produktziel

Der Nutzer soll seine Second Brains vollständig innerhalb dieser Anwendung
betrachten und bearbeiten können. Für jedes Projekt, etwa Physim, lässt sich
ein eigenes Projektgedächtnis mit derselben Grundstruktur anlegen.

Zur geplanten eigenen Oberfläche gehören der Zugang zu Zielen, aktuellem Stand,
Entscheidungen, offenen Fragen, Quellen, Wissensnotizen und Arbeitsübergaben.
Erfassung, Navigation, Suche und die Verwaltung mehrerer Projekte sollen
möglichst durch SecondBrain selbst bereitgestellt werden.

Obsidian, ein externer Markdown-Editor oder eine andere Wissensanwendung sollen
für die normale Nutzung nicht erforderlich sein. Wie die KI auf Projektkontext
zugreift und ihn pflegt, ist im weiteren Plan zu konkretisieren.

## Sprache und Abhängigkeiten

- Die Anwendung und ihr fachlicher Kern werden in C implementiert.
- Externe Bibliotheken sind ausschließlich für die UI zulässig.
- Projektverwaltung, Wissensmodell, Speicherung, Laden, Suche und sonstige
  fachliche Funktionen werden im Projekt selbst umgesetzt.
- C-Standardbibliothek und notwendige Betriebssystem-APIs bilden die Grundlage
  der plattformspezifischen Anbindung.
- Die UI-Abhängigkeiten werden später nach der Designrecherche und anhand
  ihrer Eignung für alle Zielplattformen ausgewählt.

Der vorhandene Python-Generator ist ein Strukturprototyp. Er legt noch keine
Technologie für die geplante Anwendung fest und ist keine vorgesehene
Laufzeitvoraussetzung der C-Anwendung.

## Zielplattformen und Daten

Windows, macOS und Linux sind gleichwertige Zielplattformen. Der grundlegende
Arbeitsablauf muss auf allen drei Systemen verfügbar sein. Plattformabhängige
Funktionen benötigen eine klar abgegrenzte Anbindung.

Vorhandenes Projektwissen soll bei der Weiterentwicklung erhalten bleiben.
Die vorhandenen Markdown-Vorlagen und das Physim-Beispiel dienen als Grundlage
für das Wissensmodell. Das endgültige Speicherformat und die Übernahme des
Prototyps werden vor der Implementierung festgelegt.

## UI Recherche vor dem Entwurf

Vor dem ersten UI-Entwurf wird UI-Design studiert. Die verbindliche gestalterische
Orientierung sind Apples Human Interface Guidelines und die UI-Gestaltung von
Apple. Die Recherche geht der Gestaltung von Ansichten, Navigation und
Komponenten voraus.

Die spätere Recherche soll insbesondere untersuchen:

- Informationshierarchie und Orientierung bei mehreren Projekten.
- Typografie, Abstände, Farben und verständliche Darstellung dichter Inhalte.
- Navigation, Bearbeitung, Suche sowie Rückmeldungen und Fehlerzustände.
- Tastaturbedienung, Fokus, Lesbarkeit und Barrierefreiheit.
- Übertragung der Apple-Gestaltungsrichtung auf Windows, macOS und Linux,
  einschließlich der jeweiligen Bedienkonventionen.

Erkenntnisse und Originalquellen werden zuerst dokumentiert. Daraus folgen
begründete Designprinzipien und erst anschließend konkrete UI-Entwürfe.
Die anschließende Recherche ist in [UI Recherche](UI_RECHERCHE.md) dokumentiert.
Konkrete Entwürfe werden daraus abgeleitet.

## Reihenfolge der weiteren Arbeit

1. UI-Grundlagen und Apple-Gestaltung recherchieren und dokumentieren.
2. Produktabläufe und Anforderungen an die eigene Oberfläche konkretisieren.
3. Daraus UI-Entwürfe erarbeiten und bewerten.
4. Architektur, C-Standard, Speicherformat, Buildverfahren und zulässige
   UI-Bibliotheken festlegen.
5. Einen vollständigen Arbeitsablauf in C implementieren und schrittweise erweitern.
6. Verhalten und Darstellung auf Windows, macOS und Linux tatsächlich prüfen.

Der Nutzer hat nach dem Festhalten des Plans die Umsetzung des Projekts
beauftragt. Diese Reihenfolge gilt für die nun laufende Arbeit.

## Eigenes Projektgedächtnis

- [ ] Ein eigenes Second Brain für das SecondBrain-Projekt erstellen. Die
  Anwendung soll damit auch für ihre eigene Entwicklung verwendet werden.
  Ziele, Stand, Entscheidungen, Quellen und offene Aufgaben werden darin
  nachvollziehbar gepflegt.

## Veröffentlichung und Nachweise

Jeder abgeschlossene, geprüfte Arbeitsschritt wird committet und nach
`origin` unter `Lulus792/SecondBrain` gepusht. Die Historie bleibt erhalten.
Die Regel gilt für Planung, Recherche, Design und Implementierung.

Plattformunterstützung wird anhand ausgeführter Prüfungen dokumentiert.
Die bisherigen Python-Prüfungen belegen den Strukturprototyp; sie sind kein
Nachweis für die zukünftige C-Anwendung oder deren Oberfläche.
