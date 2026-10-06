# Auf dem Weg zu 1.0

Stand: 6. Oktober 2026. Diese Liste bewertet den vorhandenen Code und die
[Distribution](DISTRIBUTION.md). Der Nutzer hat die offenen Arbeiten als Umsetzungsauftrag bestätigt. Die
Versionsnummer 1.0 darf erst nach seiner ausdrücklichen Freigabe gesetzt werden.
Die unterstützten Umgebungen und konkreten Abnahmen werden dabei festgelegt.
Die heutige Desktop-App deckt bereits Anlegen, Lesen, Bearbeiten, Suche, Quellen,
Archiv, Kontext und Tastaturbedienung ab. Ein vollständiger Release braucht darüber
hinaus einen verlässlichen Alltag und einen dauerhaften Veröffentlichungsweg.

## Vor einer stabilen Veröffentlichung

- [ ] **Umfang und Datenvertrag festlegen:** unterstützte OS-Versionen, Dateigrenzen,
  Markdown-Umfang, Verhalten bei defekten Metadaten und künftigen Vorlagenmigrationen.
  Bestandsschutz und Konflikterkennung sind implementiert; Migrationen noch nicht.
- [ ] **Wiederherstellung anbieten:** Sicherung und Wiederherstellen von Projektwissen
  mit geprüften Abbruch-, Speicherplatz- und beschädigten-Datei-Szenarien.
  Atomisches Speichern ersetzt keine Sicherung oder Versionshistorie.
  In 0.4.0 implementiert: eigene Inhaltsarchive, prüfbare Vorschau, Wiederherstellung
  und Abbruch in der App. Noch offen: native Dialogabnahme, tatsächliche volle
  Zielvolumes und Wiederherstellung nach hartem Prozessabbruch.
- [ ] **Alltagskomfort vervollständigen:** Einstellungen und letzten Arbeitsordner
  dauerhaft speichern, native Ordnerauswahl, verständlicher erster Start.
  Speicherung und native Ordnerwahl sind in 0.3.1 implementiert; erster Start,
  Systemvorgaben und interaktive native Dialogabnahme bleiben offen.
- [ ] **Barrierefreiheit abnehmen:** native Screenreader-Anbindung, Kontraste,
  Fokusreihenfolge, große Schrift, Schrift-Fallback und Systemeinstellung für
  reduzierte Bewegung. Die App bietet bereits Tastaturwege, größere Schrift und
  eigene Schalter für Transparenz und Bewegung; OS-Einstellungen werden noch nicht übernommen.
  In 0.5.0 sind native Adapter integriert und macOS-Provideraktionen gezielt geprüft.
  UIA-/AT-SPI-Clientabfragen, vollständige Text-/Dialogsemantik und tatsächliche
  VoiceOver/NVDA/Orca-Abnahme bleiben offen.
- [ ] **Leistung und Stabilität im Alltag prüfen:** große reale Wissensbasen,
  schnelle Eingabefolgen, lange Sitzungen, mehrere Displays, Skalierung und
  GPU-Treiber auf allen Zielsystemen. Die bisherigen Tests verwenden versteckte
  Fenster und den SDL-Softwarerenderer. Große Dateien und Graphgrenzen sind benannt,
  aber noch kein belastbarer Leistungsnachweis auf schwächerer Hardware.
- [ ] **Dauerhafte Release-Pakete veröffentlichen:** versionierte GitHub Releases,
  Prüfsummen, Änderungsübersicht und reproduzierbare Paketabnahme. Actions-Artefakte
  verfallen nach 30 Tagen. Signierung, macOS-Notarisierung und einfache Installation
  müssen für die gewählten Vertriebswege geklärt werden. Ein automatischer Updater
  ist optional, ein dokumentierter Updateweg notwendig.
- [ ] **Lizenz und Support klären:** Die eigene MIT-Lizenz ist auf Nutzerentscheidung festgelegt. Noch offen: alle
  Abhängigkeiten und übernommenen Anpassungen vollständig zuordnen, Fehler- und
  Sicherheitsmeldungen sowie Wartung der UI-Abhängigkeiten organisieren.
- [ ] **Endprodukt redaktionell prüfen:** kurze, natürliche Texte in App, Hilfe,
  Fehlermeldungen, Vorlagen und README; keine generischen Werbesätze oder unnötige
  Technik im normalen Bedienweg. Diese abschließende Bereinigung erfolgt auf
  Nutzerwunsch erst, wenn das Produkt vollständig ist. Entwicklungsunterlagen
  behalten ihren Zweck und ihre Nachweise.

## Mögliche spätere Erweiterungen

Chat-Anbieter, automatische KI-Pflege, Synchronisation, mobile Apps, Wiki-Links,
projektübergreifende Suche und Plugins sind bisher keine festgelegten 1.0-Voraussetzungen.
Sie brauchen eigene Anforderungen. Der derzeitige KI-Zugang über lesbare Dateien
und kopierbaren Kontext funktioniert bereits ohne Anbieteranbindung.

## Abnahmekriterium

Für jedes zugesagte System startet ein dauerhaft verfügbares Paket auf einem
frischen Rechner. Ein Nutzer legt ein Projekt an, findet und bearbeitet Wissen,
sichert und stellt es wieder her und aktualisiert die App ohne Verlust seines
Projektgedächtnisses. Derselbe zugesagte Ablauf funktioniert mit Tastatur und
unter den vereinbarten Anforderungen an Barrierefreiheit. Die Nachweise nennen
Version, System, Testumfang und verbleibende Grenzen.

## Entscheidungen des Nutzers

Am 6. Oktober ist MIT als Lizenz des eigenen Codes gewählt. Ein Apple-Developer-
Konto und ein Windows-Code-Signing-Zertifikat sind noch nicht vorhanden. Der
Signierungsweg wird vorbereitet; tatsächliche Herausgeber-Signierung und macOS-
Notarisierung können erst mit den entsprechenden Konten und Zertifikaten geprüft
werden. Entwicklungspakete werden bis dahin eindeutig als solche gekennzeichnet.
