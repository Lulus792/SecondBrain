# Projektplan für die SecondBrain Anwendung

Stand: 6. Oktober 2026. SecondBrain ist eine eigene Anwendung in C,
mit der der Nutzer und die KI projektbezogenes Wissen verwenden können.
Die Anwendung übernimmt das Anlegen, Anzeigen, Bearbeiten und Verwalten der
Projektgedächtnisse. Die folgenden Anforderungen sind verbindlich festgehalten;
UI-Recherche, Entwurf und erste C-Desktop-Version sind umgesetzt. Den aktuellen
Nachweis nennt [STATUS.md](STATUS.md).

## Produktziel

Der Nutzer soll seine Second Brains vollständig innerhalb dieser Anwendung
betrachten und bearbeiten können. Für jedes Projekt, etwa Physim, lässt sich
ein eigenes Projektgedächtnis mit derselben Grundstruktur anlegen.

Zur eigenen Oberfläche gehören der Zugang zu Zielen, aktuellem Stand,
Entscheidungen, offenen Fragen, Quellen, Wissensnotizen und Arbeitsübergaben.
Erfassung, Navigation, Suche und die Verwaltung mehrerer Projekte sollen
möglichst durch SecondBrain selbst bereitgestellt werden.

Obsidian, ein externer Markdown-Editor oder eine andere Wissensanwendung sollen
für die normale Nutzung nicht erforderlich sein. Die erste KI-Anbindung erfolgt
über kopierbaren Kontext aus gespeicherten Kerninformationen und den Zugriff
auf lesbare Projektdateien. Lesen und Pflege werden ausdrücklich beauftragt.

## Sprache und Abhängigkeiten

- Die Anwendung und ihr fachlicher Kern werden in C implementiert.
- Externe Bibliotheken sind ausschließlich für die UI zulässig.
- Projektverwaltung, Wissensmodell, Speicherung, Laden, Suche und sonstige
  fachliche Funktionen werden im Projekt selbst umgesetzt.
- C-Standardbibliothek und notwendige Betriebssystem-APIs bilden die Grundlage
  der plattformspezifischen Anbindung.
- Nach der Designrecherche wurden SDL3 und Nuklear für die UI ausgewählt.
  Die Begründung und Abgrenzung stehen in [ARCHITEKTUR.md](ARCHITEKTUR.md).

Der vorhandene Python-Generator bleibt ein Strukturprototyp. Die C-Anwendung
benötigt ihn zur Laufzeit nicht.

## Zielplattformen und Daten

Windows, macOS und Linux sind gleichwertige Zielplattformen. Der grundlegende
Arbeitsablauf muss auf allen drei Systemen verfügbar sein. Plattformabhängige
Funktionen benötigen eine klar abgegrenzte Anbindung.

Vorhandenes Projektwissen soll bei der Weiterentwicklung erhalten bleiben.
Die vorhandenen Markdown-Vorlagen und das Physim-Beispiel dienen als Grundlage
für das Wissensmodell. Die C-Anwendung verwendet UTF-8-Markdown und kleine
JSON-Metadaten und kann die vorhandenen Instanzen direkt öffnen.

## UI Recherche vor dem Entwurf

Vor dem ersten UI-Entwurf wird UI-Design studiert. Die verbindliche gestalterische
Orientierung sind Apples Human Interface Guidelines und die UI-Gestaltung von
Apple. Die Recherche geht der Gestaltung von Ansichten, Navigation und
Komponenten voraus.

Die vor dem Entwurf durchgeführte Recherche untersucht:

- Informationshierarchie und Orientierung bei mehreren Projekten.
- Typografie, Abstände, Farben und verständliche Darstellung dichter Inhalte.
- Navigation, Bearbeitung, Suche sowie Rückmeldungen und Fehlerzustände.
- Tastaturbedienung, Fokus, Lesbarkeit und Barrierefreiheit.
- Übertragung der Apple-Gestaltungsrichtung auf Windows, macOS und Linux,
  einschließlich der jeweiligen Bedienkonventionen.

Erkenntnisse und Originalquellen werden zuerst dokumentiert. Daraus folgen
begründete Designprinzipien und erst anschließend konkrete UI-Entwürfe.
Die Recherche ist in [UI Recherche](UI_RECHERCHE.md) dokumentiert.
Der [konkrete Entwurf](UI_ENTWURF.md) wurde daraus abgeleitet.

## Durchlaufene Umsetzungsreihenfolge

- [x] UI-Grundlagen und Apple-Gestaltung recherchieren und dokumentieren.
- [x] Produktabläufe und Anforderungen an die eigene Oberfläche konkretisieren.
- [x] Daraus UI-Entwürfe erarbeiten und bewerten.
- [x] Architektur, C-Standard, Speicherformat, Buildverfahren und zulässige
   UI-Bibliotheken festlegen.
- [x] Einen vollständigen Arbeitsablauf in C implementieren und schrittweise erweitern.
- [x] Verhalten und Darstellung auf Windows, macOS und Linux tatsächlich prüfen.
- [x] Entpackte Pakete ohne Python-Laufzeit auf allen drei Systemen prüfen.

Der Nutzer hat nach dem Festhalten des Plans die Umsetzung des Projekts
beauftragt. Die erste Version durchlief diese Reihenfolge; Nachweise und Grenzen
stehen im [Umsetzungsstand](STATUS.md).

## Eigenes Projektgedächtnis

- [x] Ein eigenes Second Brain für das SecondBrain-Projekt erstellen. Die
  Anwendung soll damit auch für ihre eigene Entwicklung verwendet werden.
  Ziele, Stand, Entscheidungen, Quellen und offene Aufgaben werden darin
  nachvollziehbar gepflegt.
  Die Instanz liegt unter [brains/secondbrain](../brains/secondbrain/START.md)
  und wurde in der eigenen Anwendung geladen und betrachtet.

## Neue Gestaltungsphase: Wissensgalaxie

- [x] Räumliche Wissensnetze und Sternkarten recherchieren.
- [x] Den Nutzer nach Stimmung, Sternendarstellung und Bedienelementen fragen.
- [x] Drei vergleichbare Entwürfe einschließlich glasartiger Karten vorbereiten.
- [x] Lumen-Farbpalette mit Apple-orientiertem Liquid-Glass-Look auswählen.
- [x] Die gewählte Kombination als verfeinerte Vorschau ausarbeiten und prüfen.
- [x] Materialweg, Graphumfang und Kamerabedienung für die C-Integration konkretisieren.
- [x] Lumen-Sternkarte und eigene Glasdarstellung in C implementieren.
- [x] Vorhandene Arbeitsabläufe in schwebende Karten integrieren.
- [x] Sichtbaren Fokus und vollständig per Tastatur bedienbare Abläufe implementieren.
- [x] Beide Bedienwege lokal in Release prüfen, einschließlich kleinem Fenster
  und 150 Prozent Schriftgröße.
- [ ] Neue UI und Tastaturwege mit entpackten Paketen auf allen drei Plattformen abnehmen.

Referenzen, Antworten und der Status der Vorschauen stehen in
[UI_GALAXIE.md](UI_GALAXIE.md). Die Umsetzung wurde anschließend
ausdrücklich beauftragt. Sie steht in der C-App; den jeweils belegten Prüfstand
nennen [STATUS.md](STATUS.md) und [UI_TASTATUR.md](UI_TASTATUR.md).

## Laufende Veröffentlichung und Nachweise

Jeder abgeschlossene, geprüfte Arbeitsschritt wird committet und nach
`origin` unter `Lulus792/SecondBrain` gepusht. Die Historie bleibt erhalten.
Die Regel gilt für Planung, Recherche, Design und Implementierung.

Plattformunterstützung wird anhand ausgeführter Prüfungen dokumentiert.
Die bisherigen Python-Prüfungen belegen den Strukturprototyp; sie sind kein
Nachweis für die C-Anwendung oder deren Oberfläche. Dafür bestehen eigene
Kern-, UI-, Desktop- und Paketprüfungen.
