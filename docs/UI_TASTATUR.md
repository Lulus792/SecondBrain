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
  sofort zur nächsten Notiz in der Karte. Ein offener Entwurf bleibt durch den
  Schutzdialog gesichert. Enter kann den aktuellen Stern erneut öffnen.
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

Implementiert ist ein eigener C-Materialrenderer mit Lichtkanten und Brechung des
Sternhintergrunds. Er verwendet SDL als UI-Bibliothek und funktioniert auch auf
älteren Macs sowie Windows und Linux. Dies ist eine optische Nachbildung;
Apples native macOS-26-Materialimplementierung wird damit nicht behauptet.
Die vorhandenen Modelle für Speicherung und Änderungsschutz bleiben maßgeblich.

## Prüfung

Am 6. Oktober bestehen lokal alle sieben Prüfungen in Release sowie mit
AddressSanitizer und UndefinedBehaviorSanitizer auf Intel macOS 14.6.1.
Der zusätzliche Tastaturdurchlauf prüft 85 Aussagen über echte SDL-Ereignisse:
Erstellen, Bereichswahl, Bearbeiten, Speichern, Tabulator, Undo/Redo, Kamera,
Sternwahl ohne sofortiges Öffnen, Schutzdialog, Suche, lokale Quelle, Kontext,
Konfliktkopie ohne Überschreiben des Originals, Einstellungen, kleine Darstellung mit 150 Prozent Schrift, Hilfe, Archiv,
Arbeitsordner, Projektwahl und Beenden. Eine Notiz mit 300 Quellenlinks prüft die Erreichbarkeit des letzten Links und
die Fokuswiederherstellung nach der Quellenansicht. Die Prüfung kontrolliert auch
die aktive SDL-Texteingabe in Editor und frisch geöffneten Formularen. Der damalige Durchlauf injizierte keine Mausereignisse.

Der bisherige Bedienweg besteht separat mit 87 Aussagen. Die Materialprüfung
weist nach, dass Glas auf geänderte Sterne reagiert und reduzierte Transparenz
seinen Inhalt von diesen Änderungen abschirmt. Der Pakettest führt beide
Bedienwege aus dem tatsächlich entpackten und verschobenen Paket aus.
Die [Abnahme zu 5534ff0](https://github.com/Lulus792/SecondBrain/actions/runs/37466622105) besteht mit allen 18 Jobs.
Alle sechs nativen Desktop-Jobs führen sieben Prüfungen aus; alle drei entpackten
Release-Pakete bestehen zusätzlich beide Bedienwege auf Windows x64, macOS ARM64
und Linux x64. Die CI verwendet versteckte native Fenster mit SDL-Softwarerenderer.

Tasten im Editor: Tab verlässt das Feld; Ctrl+I fügt einen Tabulator ein.
Das gilt auch auf macOS, wo Ctrl ausdrücklich die Control-Taste bezeichnet.
Die übrigen App-Kürzel verwenden Command auf macOS beziehungsweise Control
auf Windows und Linux. Es gibt keine Tastaturfalle im Mehrzeileneditor.

## Verfeinerung: direkte Navigation und Bewegung

Der neue Nutzerauftrag ersetzt die frühere Trennung zwischen Sternwahl und Öffnen.
Pfeile öffnen direkt, ohne zusätzliche Bestätigung. Bei einem Entwurf entscheidet
weiterhin der Schutzdialog über Speichern, Verwerfen oder Abbrechen.

Apples [Motion](https://developer.apple.com/design/human-interface-guidelines/motion)
wurde am 6. Oktober erneut gelesen: Bewegung gezielt einsetzen, abbrechbar und
optional machen. Kamera und Scrollposition nähern sich ihrem Ziel anhand verstrichener
Zeit. Neue Eingaben ändern das Ziel während des Übergangs; Ziehen folgt der Hand direkt.
Die Kamera folgt der gewählten Notiz mit einer kleinen Verschiebung. „Bewegung
reduzieren“ in den Einstellungen schaltet die Übergänge ab. Die sichtbare Auswahl
und der Dokumentinhalt wechseln unabhängig davon sofort. Die Betriebssystem-
Einstellung für reduzierte Bewegung wird bisher nicht automatisch übernommen.

Suchhinweise werden vertikal an der Schrift ausgerichtet. Kompakte Schaltflächen
bekommen genug Raum oberhalb und unterhalb ihrer Beschriftung. Die vorhandenen
Materialien und die Lumen-Farbpalette bleiben erhalten.

Die lokale Release-Prüfung von 0.2.1 besteht mit allen sieben Tests. Der reine
Tastaturweg prüft 99 Aussagen ohne injizierte Mausereignisse; der Maus-Bedienweg
prüft 93 Aussagen einschließlich halber Mausradschritte und Richtungswechsel.

Dieselben sieben Tests bestehen mit AddressSanitizer/UndefinedBehaviorSanitizer.
Der [Plattformlauf zu 29b15d1](https://github.com/Lulus792/SecondBrain/actions/runs/37479319260) besteht mit allen 18 Jobs einschließlich beider Bedienwege aus den entpackten
Windows-, macOS- und Linux-Release-Paketen.

## Überschriften ab 0.9.1

Bei Fokus auf der Leseansicht springt Alt+Bild ab zur nächsten, Alt+Bild auf
zur vorherigen Überschrift. Wiederholte Eingaben wechseln sofort das Ziel;
der Lesebereich bewegt sich über die bestehende Scrollanimation dorthin.
Reduzierte Bewegung überspringt die Bewegung. Normales Scrollen setzt die
Gliederungsposition zurück. Editor und Sternkarte behalten ihre eigenen Tasten.
Native Scroll-into-view-Anfragen verwenden dieselben Bytepositionen im
Dokument. Dokumentwechsel invalidieren ausstehende Abschnittsanfragen.
