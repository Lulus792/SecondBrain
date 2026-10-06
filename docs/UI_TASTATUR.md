# Tastatur und räumliche Oberfläche

Stand: 6. Oktober 2026. Der Nutzer beauftragt die Umsetzung der ausgewählten
Lumen-/Liquid-Glass-Richtung und eine durchgängige Bedienung per Tastatur.

## Eingesehene Grundlagen

Apples [Keyboards](https://developer.apple.com/design/human-interface-guidelines/keyboards)
und [Focus and selection](https://developer.apple.com/design/human-interface-guidelines/focus-and-selection)
wurden einschließlich der öffentlichen Dokumentationsdaten gelesen. Abgeleitet:
übliche Speichern-/Textkürzel erhalten, sichtbarer Fokus, vorhersehbare Reihenfolge,
Fokus und Öffnen unterscheiden, Escape zum Abbrechen. Da die eigene SDL-Oberfläche
keine native Full-Keyboard-Access-Brücke bietet, setzt sie die Wege selbst um.

## Verbindlicher Bedienvertrag für die Umsetzung

- Tab / Umschalt+Tab erreicht alle verfügbaren Bedienelemente in einer stabilen
  Reihenfolge. Dialoge begrenzen den Fokus auf ihren Inhalt; Schließen stellt ihn
  wieder her. Enter / Leertaste aktiviert fokussierte Schaltflächen.
- F6 wechselt zwischen Werkzeugen, Sternkarte und Dokument. Pfeile wechseln
  Sterne, ohne dabei eine Notiz zu öffnen; Enter öffnet den fokussierten Stern.
- Die Kamera ist über Tastatur und sichtbare Schaltflächen bedienbar. Eine
  Listenansicht und die Suche bieten direkten Zugriff bei großen Wissensbasen.
- Texteingabe behält Cursor-, Auswahl-, Zwischenablage- und Undo-Funktionen.
  Tab verlässt ein Eingabefeld; eine ausdrückliche alternative Taste fügt im
  Mehrzeileneditor einen Tabulator ein. Kein Tastaturweg darf darin gefangen sein.
- Erstellen, Ordner öffnen, Bereich wählen, Quellen lesen, Kontext kopieren,
  Einstellungen, Konfliktkopie, Archiv und Schutzdialog sind per Tastatur erreichbar.
- Lesebereiche lassen sich per Tastatur scrollen. Hilfe erklärt die tatsächlich
  implementierten Wege. Reduzierte Transparenz hält Inhalt bei Bedarf ruhig.

## Daten und Material

Die Sternkarte verwendet Dokumente des geöffneten Projekts. Foldergruppen erklären
die räumliche Anordnung; Verbindungslinien entstehen ausschließlich aus vorhandenen
internen Markdown-Verweisen. Externe Quellen sind keine erfundenen Sternkanten.
Das erste Graphmodul ist in C implementiert; Pfade, Fragmentverweise, Prozentkodierung,
Bild-/Codeausschlüsse und doppelte Links werden geprüft.

Geplant ist ein eigener C-Materialrenderer mit Lichtkanten und Brechung des
Sternhintergrunds. Er verwendet SDL als UI-Bibliothek und funktioniert auch auf
älteren Macs sowie Windows und Linux. Dies ist eine optische Nachbildung;
Apples native macOS-26-Materialimplementierung wird damit nicht behauptet.
Die vorhandenen Modelle für Speicherung und Änderungsschutz bleiben maßgeblich.

## Prüfung

Am 6. Oktober bestehen lokal die sechs C-/UI-Prüfungen einschließlich Graphprüfung
mit AddressSanitizer und UndefinedBehaviorSanitizer. Die neue räumliche Oberfläche
und vollständige Fokusführung sind zu diesem Stand noch in Umsetzung.
