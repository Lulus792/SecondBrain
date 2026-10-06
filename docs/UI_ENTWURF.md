# Oberflächenentwurf für SecondBrain

Der Entwurf folgt der dokumentierten [UI Recherche](UI_RECHERCHE.md).
Die Oberfläche verbindet Projektwahl, Dokumentauswahl und die aktuelle Notiz.

## Hauptfenster

Die linke Seitenleiste zeigt Projekte und Wissensbereiche. Die mittlere Spalte
zeigt die zugehörigen Dokumente und eine Suche über Titel und Inhalte. Die große
rechte Fläche dient dem Lesen und Bearbeiten. Das aktuelle Projekt und Dokument
bleiben erkennbar. Bei kleinen Fenstern kann die Seitenleiste ausgeblendet werden.

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

Helle und dunkle Flächen, ruhige Trennlinien, klare Überschriften und ein blauer
Akzent setzen die Apple-Gestaltungsrichtung um. Primäre Inhalte erhalten deutlich
mehr Raum als Bedienelemente. Fensterrahmen und Fensterknöpfe stellt das System.
Schriftgröße und Darstellung sind anpassbar.

Beschriftete Aktionen und dokumentierte Tastenkombinationen ergänzen die Maus.
Command wird unter macOS verwendet, Control unter Windows und Linux. Sichtbare
Speicherzustände verwenden Text zusätzlich zur Farbe. Die tatsächlichen
Barrierefreiheitsgrenzen des UI-Backends werden in der Abnahme benannt.

## Prüfung des Entwurfs

Die tatsächliche Anwendung wird in mindestens zwei Fenstergrößen und mit
vergrößerter Schrift betrachtet. Der vollständige Ablauf umfasst Anlegen,
Lesen, Bearbeiten, Speichern, Wiederöffnen, Suchen, Quellenansicht, Kontext und
Projektwechsel. Die Prüfung erfolgt auf den drei Zielplattformen.
