# Oberflächenentwurf für SecondBrain

Der Entwurf folgt der dokumentierten [UI Recherche](UI_RECHERCHE.md).
Die Oberfläche verbindet Projektwahl, Dokumentauswahl und die aktuelle Notiz.

## Hauptfenster

Die ausgewählte Lumen-Sternkarte bildet den Hintergrund. Die obere schwebende
Glasleiste enthält Projektwahl, Suche, Liste und Erstellen. Kleine beschriftete
Lichtpunkte zeigen echte Dokumente des geöffneten Projekts. Die räumlichen
Gruppen entsprechen Wissensbereichen; Linien stehen für vorhandene interne
Markdown-Verweise. Die Kamera lässt sich drehen, verschieben und zoomen.

Eine Auswahl öffnet die schwebende Lesekarte rechts. Bearbeiten vergrößert sie;
die Dokumentliste bleibt daneben zugänglich. Die Karte lässt sich schließen,
während ihr Entwurf im Modell erhalten bleibt. Kleine Fenster und große Schrift
verwenden umgebrochene Werkzeuge und scrollbare Karten. Dialoge liegen darüber.
Die [Designrecherche](UI_GALAXIE.md) enthält Referenzen und ausgewählte Vorschau.

Das Anlegen eines Projekts fragt nach einem Namen, einer portablen Kennung und
optional dem Ordner des eigentlichen Projekts. Eine neue Notiz erhält einen
Titel und einen Wissensbereich. Die Grundstruktur entsteht durch die Anwendung.

## Lesen und Bearbeiten

Der Lesemodus stellt Überschriften, Absätze, Listen und Code verständlich dar.
Links auf lokale Markdown-Quellen lassen sich im selben Fenster betrachten.
Die Bearbeitung verwendet Markdown, damit Dateien für KI und andere Werkzeuge
zugänglich bleiben. Lesen und Bearbeiten sind zwei Ansichten derselben Datei.

Änderungen werden ausdrücklich gespeichert. Ein Dokument- oder Projektwechsel
sowie das Beenden mit ungespeicherten Änderungen bieten Speichern, Verwerfen und
Abbrechen. Externe Dateiänderungen werden vor dem Speichern erkannt und müssen
geklärt werden. Fehlermeldungen bleiben sichtbar und erhalten den bearbeiteten Text.

## Gemeinsame Arbeit mit der KI

Ein Kontextbefehl stellt die Kerninformationen des Projekts zusammen und macht
sie kopierbar. Die lesbaren Dateien bleiben die gemeinsame Wissensbasis.
Die erste direkte Verbindung zur KI erfolgt über diesen Kontext und Dateizugriff;
ein bestimmter KI-Anbieter wird nicht vorausgesetzt.

## Gestaltung und Bedienung

Die dunkle Lumen-Palette, optische Brechung des Sternhintergrunds, gewölbte
Lichtkanten und klare Typografie setzen die ausgewählte Gestaltungsrichtung um.
Das Material wird im eigenen C-Renderer berechnet und bei unveränderter Szene
wiederverwendet. Es ist eine optische Nachbildung. Reduzierte Transparenz und
eine helle Variante stehen in der Darstellungsauswahl bereit. Primäre Inhalte erhalten deutlich
mehr Raum als Bedienelemente. Fensterrahmen und Fensterknöpfe stellt das System.
Schriftgröße und Darstellung sind anpassbar.

Beschriftete Aktionen, sichtbarer Fokus und vollständige Tastaturwege ergänzen
die Maus. Der [Bedienvertrag](UI_TASTATUR.md) beschreibt Reihenfolge, Kartenwahl,
Texteingabe, Scrollen und Dialoge.
Command wird unter macOS verwendet, Control unter Windows und Linux. Sichtbare
Speicherzustände verwenden Text zusätzlich zur Farbe. Die tatsächlichen
Barrierefreiheitsgrenzen des UI-Backends werden in der Abnahme benannt.

## Prüfung des Entwurfs

Die tatsächliche Anwendung wird in mindestens zwei Fenstergrößen und mit
vergrößerter Schrift betrachtet. Der vollständige Ablauf umfasst Anlegen,
Lesen, Bearbeiten, Speichern, Wiederöffnen, Suchen, Quellenansicht, Kontext und
Projektwechsel. Die Prüfung erfolgt auf den drei Zielplattformen.
