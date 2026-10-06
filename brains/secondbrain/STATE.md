# SecondBrain: aktueller Stand

Stand: 6. Oktober 2026. Maßgebliche Details und historische Nachweise stehen im
[Umsetzungsstand](../../docs/STATUS.md) und in [Plattformprüfungen](../../docs/PLATTFORMEN.md).
Diese Übersicht hält den aktuellen Einstieg klein.

## Implementiert und belegt

- Eigene C17-App mit Projekt-/Notizerstellung, Lesen, Bearbeiten, Suche, Quellen,
  Archiv und kopierbarem Kontext; Entwürfe und erkannte Speicherkonflikte sind geschützt.
- Lumen-Sternkarte, eigene Glasdarstellung und Icons, direkte Pfeilnavigation,
  Raumfahrt, begrenztes weiches Scrollen, Menüpfeile und große Leseansicht.
- In 0.3.1: dauerhafte Darstellung, Fenster und letzte Projektposition; native
  Ordnerauswahl integriert, alte Callback-Antworten geschützt.
- [0.3.1-Abnahme zu 259fca1](https://github.com/Lulus792/SecondBrain/actions/runs/37499338633):
  18 erfolgreiche Jobs. Je zehn Desktoptests in Debug/Release und entpackte Pakete
  bestehen auf Windows x64, macOS ARM64 und Linux x64. Der anfängliche Linux-
  Größenfehler wurde durch Warten auf tatsächliche Fenstergeometrie behoben.
- Lokal auf Intel macOS 14.6.1 bestehen Release-, Sanitizer- und Paketprüfungen;
  126 Maus-/105 Tastaturaussagen und Prozessneustarts sind enthalten. Die App
  liegt unter dist/SecondBrain/secondbrain.app. Kleine Ansichten wurden betrachtet.
- Dieses eigene Projektgedächtnis wurde mit dem C-Kern angelegt und in der App
  gelesen; seine Quellen verbinden Auftrag, Originale und Nachweise.

## Laufende Arbeit: Sicherung und Wiederherstellung

Der eigene C-Kern implementiert Inhaltsarchive mit SHA-256, vollständige Prüfung,
exklusives Veröffentlichen, neuen Zielordnernamen und Aufräumen bei Abbruch/Fehler.
Binäre Anhänge und leere Ordner sind enthalten; externe Projektverknüpfungen werden
nicht kopiert. Bestehende Sicherungen und Projektordner werden nicht ersetzt.
Der Entwicklungs-CLI bietet backup, inspect und restore.
[Vertrag und Grenzen](../../docs/SICHERUNG.md).

Die gezielten lokalen Prüfungen umfassen komplette Rundreise, Unicode, bekannte
SHA-256-Vektoren, beschädigte/unzulässige Archive, Abbruch, Quellenkonflikte und
simulierten vollen Datenträger. Produktions-CLI wird separat über Prozesse geprüft.
Die Desktop-Anbindung ist in 0.4.0 umgesetzt: Aktionen/Projekte bieten Sichern
und Wiederherstellen, Hintergrundarbeit mit Abbruch, prüfbare Vorschau und
erhaltene Entwürfe. Der Tastaturdurchlauf umfasst 75 Aussagen und eine
8-MiB-Datei. Die [0.4.0-Abnahme zu d2d9c2b](https://github.com/Lulus792/SecondBrain/actions/runs/37508856383) besteht mit 18 Jobs:
14 Desktoptests je Debug/Release auf Windows x64, macOS ARM64 und Linux x64.
Alle entpackten Pakete bestehen Sicherungs-UI, beide bisherigen Bedienwege,
Einstellungsneustart und Produktions-CLI. Lokal bestehen Paket und gezielte
Sanitizer-Nachprüfung nach dem korrigierten Clipboard-Prüfbetrieb.

## Laufende Arbeit: native Zugänglichkeit

0.5.0 bindet AccessKit ausschließlich in der UI an alle drei Plattformadapter an.
Der native macOS-Provider wird tatsächlich über NSAccessibility abgefragt und
bedient: Schaltflächen, Texteingaben, Notizerstellung, Speichern, Auswahl und Lesen.
Alte Aktionen nach Dokumentwechsel werden verworfen. Die [Plattformprüfung zu 0c9fc8b](https://github.com/Lulus792/SecondBrain/actions/runs/37512645717) besteht mit 18 Jobs,
15 Desktoptests je Debug/Release und drei entpackten Paketen. Lokal bestehen
alle 15 Release- und ASan/UBSan-Tests. Das entpackte Intel-Paket startet
aus einem Unicode-Pfad; dist/SecondBrain ist auf 0.5.0 aktualisiert. Der [Clientlauf zu 8ff50ee](https://github.com/Lulus792/SecondBrain/actions/runs/37515016302) besteht ebenfalls mit allen 18 Jobs:
UIA und AT-SPI werden tatsächlich für Namen/Rollen, Unicode-Eingabe, Notizerstellung,
Editor und Speichern benutzt. Menschliche Screenreader-Abnahme und vollständige
Text-/Dialogsemantik bleiben offen. [Umfang und Grenzen](../../docs/BARRIEREFREIHEIT_PLAN.md).

Die Erweiterung 0.5.1 übernimmt sichtbare Dialog-/Hilfetexte und Entwurfswarnungen
in den nativen Baum. Die [korrigierte Plattformprüfung zu 9eeb402](https://github.com/Lulus792/SecondBrain/actions/runs/37518758088) besteht mit 18 Jobs,
15 Desktoptests je Debug/Release und drei Paketen. Lokal bestehen Release-Lauf,
Sanitizer-Gesamtlauf des Zwischenstands und gezielte abschließende native/Tastatur/
Sicherungs-Nachprüfungen. dist/SecondBrain ist auf 0.5.1 aktualisiert. Vollständige Dokumentstruktur, Unicode-Textnavigation,
Fortschrittsansagen und menschliche assistive Bedienung bleiben offen.

0.5.2 korrigiert die Linux-Cache-Signalstruktur. Die [Abnahme zu 8bcc048](https://github.com/Lulus792/SecondBrain/actions/runs/37522537774)
besteht mit 18 Jobs, 15 Desktoptests je Debug/Release und drei Paketen. Die echte
Wire-Prüfung zeigt in beiden Linux-Konfigurationen keine falsche Signatur und eine
gültige Sammelantwort; der Client verarbeitet Events ohne Cacheleerung. Linux baut
diese UI-Abhängigkeit aus festgelegter Quelle mit Cargo/Rust ab 1.87. Anwendung und
Kern bleiben C; Pakete brauchen die Toolchain nicht. dist/SecondBrain ist auf 0.5.2.
Weitere Eventtypen und menschliche Screenreader-Abnahme bleiben offen.

## Auftrag und verbleibende Abnahmen

Das vollständige [Ziel vor 1.0](../../docs/RELEASE.md) bleibt aktiv: Sicherungs-UI,
native Screenreader, erster Start, OS-Vorgaben, Daten-/Leistungsabnahme,
dauerhafte Distribution, Support und abschließende Produkttexte. Die Version 1.0
bleibt bis zur ausdrücklichen Nutzerfreigabe gesperrt. Eigener Code: MIT.
Apple-Developer-Konto und Windows-Signaturzertifikat sind noch nicht vorhanden.

Die bisherigen UI-Nachweise verwenden SDL-Ereignisse, versteckte native Fenster
und Softwarebilder. Tatsächliche OS-Dialog-, VoiceOver-/NVDA-/Orca-, GPU-/Display-
und Langzeitabnahmen fehlen. Markdown und Schriftabdeckung bleiben begrenzt.
Automatische Synchronisation und Chat-Anbieter sind optionale spätere Funktionen.
