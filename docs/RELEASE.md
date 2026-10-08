# Auf dem Weg zu 1.0

Stand: 8. Oktober 2026. Diese Liste bewertet den vorhandenen Code und die
[Distribution](DISTRIBUTION.md). Der Nutzer hat die offenen Arbeiten als Umsetzungsauftrag bestätigt. Die
Versionsnummer 1.0 darf erst nach seiner ausdrücklichen Freigabe gesetzt werden.
Die unterstützten Umgebungen und konkreten Abnahmen werden dabei festgelegt.
Die heutige Desktop-App deckt bereits Anlegen, Lesen, Bearbeiten, Suche, Quellen,
Archiv, Kontext und Tastaturbedienung ab. Ein vollständiger Release braucht darüber
hinaus einen verlässlichen Alltag und einen dauerhaften Veröffentlichungsweg.

## Vor einer stabilen Veröffentlichung

- [ ] **Umfang und Datenvertrag festlegen:** unterstützte OS-Versionen, Dateigrenzen,
  Markdown-Umfang, Verhalten bei defekten Metadaten und künftigen Vorlagenmigrationen.
  Bestandsschutz und Konflikterkennung sind implementiert. Der
  [Datenvertrag](DATENVERTRAG.md) beschreibt ab 0.7.1 Metadatenversionen, Altformat,
  Textgrenzen und NUL-Abweisung mit erhaltenen Originalbytes. In 0.7.2 erhält die
  Projektwahl einzelne Fehler mit Grund und erneutem Prüfen; ihre native Plattform-
  und Paketabnahme besteht. OS-Mindestversionen und zukünftige Migrationen bleiben offen.
- [ ] **Wiederherstellung anbieten:** Sicherung und Wiederherstellen von Projektwissen
  mit geprüften Abbruch-, Speicherplatz- und beschädigten-Datei-Szenarien.
  Atomisches Speichern ersetzt keine Sicherung oder Versionshistorie.
  In 0.4.0 implementiert: eigene Inhaltsarchive, prüfbare Vorschau, Wiederherstellung
  und Abbruch in der App. Ab 0.9.2 bestehen sechs echte Prozessabbruch-/Neustartfälle
  auf den nativen CI-Systemen einschließlich Windows/Linux und zwei ENOSPC-Fälle
  auf einem begrenzten HFS+-Volume. Noch offen: native Dialogbedienung, volle
  Windows-/Linux-Zielvolumes, weitere Dateisysteme und physische Persistenzabnahme.
  [Konkreter Umfang](PLATTFORMEN.md), [Fehlerszenarien](SICHERUNG.md).
- [ ] **Alltagskomfort vervollständigen:** Einstellungen und letzten Arbeitsordner
  dauerhaft speichern, native Ordnerauswahl, verständlicher erster Start.
  Speicherung und native Ordnerwahl sind in 0.3.1 implementiert; Systemvorgaben
  sind in 0.6.0 im dokumentierten Umfang angebunden. Der leere Einstieg ist in
  0.7.0 implementiert und auf allen drei CI-Plattformen sowie in Paketen geprüft.
  Interaktive native Dialog- und menschliche Bedienabnahmen bleiben offen.
  0.9.26 ergänzt die [geordnete Texteingabe](IME.md) über Fokuswechsel und
  den schnellen Neuaufbau nach Formularbefehlen; native Abnahme folgt gesondert.
- [ ] **Barrierefreiheit abnehmen:** native Screenreader-Anbindung, Kontraste,
  Fokusreihenfolge, große Schrift, Schrift-Fallback und Systemeinstellung für
  reduzierte Bewegung. Die App bietet bereits Tastaturwege, größere Schrift und
  eigene Schalter für Transparenz und Bewegung. In 0.6.0 ergänzen bekannte native
  Vorgaben die eigene Auswahl, ohne sie dauerhaft zu verändern.
  In 0.5.0 sind native Adapter integriert und macOS-Provideraktionen gezielt geprüft.
  UIA-/AT-SPI-Clientabfragen bestehen inzwischen auf den nativen CI-Systemen.
  In 0.5.1 bestehen native Dialog-/Hilfetexte auf allen drei Systemen. Dokumentstruktur,
  Unicode-Textgeometrie und tatsächliche VoiceOver/NVDA/Orca-Abnahme bleiben offen.
  In 0.9.0 ergänzt eine eigene SDL_ttf-Anbindung HarfBuzz, FreeType und
  Ersatzschriften; lokale Raster-/Text-/Paketprüfungen bestehen. Gemischte
  Schreibrichtungen und präzise visuelle Eingabegeometrie bleiben in
  [TEXTDARSTELLUNG.md](TEXTDARSTELLUNG.md) offen. 0.9.25 ergänzt
  die [vorläufige IME-Komposition](IME.md); reale native Eingabemethoden,
  Bidi und visuelle/native Zeichenrechtecke bleiben gesonderte Abnahmen. 0.9.8 implementiert und prüft
  [vollständige Graphem-Eingaben](GRAPHEME.md); 0.9.9 ergänzt die
  [Emoji-Schrift und Schriftläufe](EMOJI.md), deren Abschlussprüfung läuft. Ab 0.9.1 veröffentlicht
  die Leseansicht strukturierte Blöcke und bietet Abschnittssprünge;
  [Dokumentstruktur](DOKUMENTSTRUKTUR.md) nennt Umfang und verbleibende Semantik.
  0.9.4 erweitert die gemeinsamen [Blockregeln](MARKDOWN.md); vollständige
  Container-/Inline-Regeln und Listensemantik bleiben offen. 0.9.11 ergänzt
  gemeinsame [Hervorhebungen](INLINE_STILE.md); 0.9.14 ergänzt
  [Zeichenreferenzen und Mathematikglyphen](ENTITIES.md); 0.9.15 ergänzt
  [E-Mail-Autolinks](AUTOLINKS.md). [Referenzlink-Integration](REFERENZLINKS.md)
  ist ab 0.9.16 für die vorhandenen Blöcke implementiert. 0.9.17 ergänzt den
  gemeinsamen [Containerbaum und Listen-/Zitatdarstellung](CONTAINER_UI.md),
  einschließlich Definitionen darin. Vollständige Markdown-/assistive Abnahme
  bleibt offen. 0.9.12 bindet
  [native Textstile](NATIVE_TEXTSTILE.md) an. Die neuen Plattformnachweise und
  die dokumentierten weiteren Textregeln bleiben offen. 0.9.6 ergänzt
  [Tabellen](TABELLEN.md). Mac-Matrix ist ab 0.9.27 lokal geprüft; Windows
  Grid/Table besteht ab 0.9.29 nativ in Debug/Release und im entpackten Paket.
  AT-SPI Table/TableCell besteht ab 0.9.32 nativ in Debug/Release und im
  entpackten Linux-Paket. 0.9.33 begrenzt die Cache-Indizes großer Tabellen;
  die neue native Cache- und Paketabnahme besteht. Menschliche Tabellenbedienung bleibt gesondert offen.
  Die Cache-Signalstruktur ist in 0.5.2 korrigiert und mit echtem Linux-Clientcache
  geprüft; weitere Eventtypen bleiben gesonderte Abnahmen. Systemvorgaben bestehen
  in 0.6.0 auf allen drei CI-Systemen; reale Einstellungswechsel, Windows-Custom-
  High-Contrast-Paletten und vollständige Kontrastmessung bleiben offen. In 0.6.1
  werden funktionale Sternkerne und Kanten mit Rastermessung nachgeprüft.
- [ ] **Leistung und Stabilität im Alltag prüfen:** große reale Wissensbasen,
  schnelle Eingabefolgen, lange Sitzungen, mehrere Displays, Skalierung und
  GPU-Treiber auf allen Zielsystemen. Die bisherigen Tests verwenden versteckte
  Fenster und den SDL-Softwarerenderer. Große Dateien und Graphgrenzen sind benannt,
  aber noch kein belastbarer Leistungsnachweis auf schwächerer Hardware.
- [ ] **Dauerhafte Release-Pakete veröffentlichen:** versionierte GitHub Releases,
  Prüfsummen, Änderungsübersicht und reproduzierbare Paketabnahme. Actions-Artefakte
  verfallen nach 30 Tagen. Signierung, macOS-Notarisierung und einfache Installation
  müssen für die gewählten Vertriebswege geklärt werden. Ein automatischer Updater
  ist optional, ein dokumentierter Updateweg notwendig. Die dauerhafte Vorabversion
  [v0.7.3](https://github.com/Lulus792/SecondBrain/releases/tag/v0.7.3) ist am
  7. Oktober veröffentlicht: vier geprüfte Plattformarchive und SHA256SUMS,
  Uploads vor Veröffentlichung erneut verifiziert. Signierung und die vollständige
  Abnahme auf frischen Nutzerrechnern bleiben offen.
- [ ] **Lizenz und Support klären:** Die eigene MIT-Lizenz ist auf Nutzerentscheidung festgelegt. Noch offen: alle
  Abhängigkeiten und übernommenen Anpassungen vollständig zuordnen, Fehler- und
  Sicherheitsmeldungen sowie Wartung der UI-Abhängigkeiten organisieren. Ab 0.9.3
  stehen mitgelieferte Originaltexte direkt in der [App-Lizenzansicht](LIZENZEN.md).
  Fehler-/Vorschlagsformulare, [Supporthinweise](../SUPPORT.md),
  [Beitragsregeln](../CONTRIBUTING.md) und ein
  [Ablauf für Abhängigkeitsupdates](ABHAENGIGKEITEN_WARTUNG.md) sind vorbereitet.
  Vertraulicher Sicherheitskanal und vollständige transitive Lizenzprüfung bleiben offen.
  Ab 0.9.20 sind 113 Cargo-Komponenten und zusätzliche SDL-/HarfBuzz-Hinweise
  [inventarisiert](LIZENZ_INVENTUR.md) und mitgeliefert. Der Rust-/Systemruntime-
  Abgleich bleibt offen; dieser Teilschritt schließt die Release-Aufgabe nicht.
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

## Ergänzende Nachprüfung vom 7. Oktober

- [x] Den letzten gültigen Desktop-Graph mit stabilen Dokumentkennungen und
  zugehörigen Beschriftungen erhalten, wenn die aktuelle Notizinventur geändert
  oder unlesbar ist. Der Kern erhält seinen Graph bereits; die Desktopdarstellung
  darf keine alten Indizes auf neue Notizen anwenden. Ab 0.9.19 ist die
  [zusammenhängende Desktop-Inventur](STERNKARTEN_BESTAND.md) implementiert;
  lokale Gesamt-/Schlussprüfung und gezielte ASan/UBSan bestehen. Neue
  Plattform- und menschliche Abnahmen bleiben gesondert offen.
- [ ] Die neue [Interaktionspolitur](INTERAKTION.md) auf den tatsächlichen
  Zielsystemen und mit menschlicher Bedienung/verschiedenen Geräten abnehmen.

## Textfortschritt vom 8. Oktober

Ab 0.9.34 benutzt die formatierte Leseansicht den gemeinsamen Bidi-/Script-/
Glyphenplan für Umbruch und Rasterung. Lokale Gesamtprüfung, gezielter
Sanitizer und UI-Build ohne Tests bestehen; [Nachweise](STATUS.md).
Die offene Textaufgabe betrifft weiterhin Editor, Suche/Formulare/einfache Labels,
visuelle Carets/Auswahl/IME, native Zeichenrechtecke und reale assistive Abnahme.
Die neue Reader-Integration benötigt außerdem ihre eigene Plattform-/Paketabnahme.

## Editorfortschritt vom 8. Oktober

Ab 0.9.35 verwendet die produktive Eingabe einen gemeinsamen Glyphen-/Cursorplan
für Darstellung, Auswahl, Maus, Pfeile und IME-Markierung. Die vorhandene
logische Speicherung und Undo bleiben erhalten. [Umfang/Nachweise](EDITORGEOMETRIE.md).
Native Zeichenrechtecke, einfache Label-/Schaltflächentexte, Unicode-Wortregeln,
große reale Dateien sowie reale Eingabe-/Screenreader-Abnahme bleiben offen.
Die neue Plattform-/Paketabnahme wird in STATUS gesondert dokumentiert.

## Einfache Textdarstellung vom 8. Oktober

Ab 0.9.36 sind einfache Textkommandos, Fontmessung und graphemgebundene Kürzung
an den gemeinsamen Glyphenplan angeschlossen. Lokale Gesamt-/Pixel-/Sanitizer-
Prüfungen bestehen; [Umfang und Grenzen](PLAIN_TEXT.md). Der allgemeine
Widgetumbruch benötigt weiterhin zusammenhängenden Absatzkontext und gleiche
Höhen-/Rasterpläne. Native Textgeometrie und reale assistive/IME-Abnahme bleiben
offen. Die neue Plattform-/Paketabnahme folgt getrennt in STATUS.

## Allgemeiner Umbruch vom 8. Oktober

Ab 0.9.37 behalten einfache umgebrochene Texte und Codeblöcke den ursprünglichen
Absatzkontext. Messung und Rasterung teilen den Layoutplan; Widgetflächen clippen
ihre eigene Zeichnung. Lokale Gesamt-, unabhängige Geometrie-/Pixel- und gezielte
Sanitizerprüfungen bestehen; [Umfang](WRAPPED_TEXT.md). Native Zeichenrechtecke,
Unicode-Wortbefehle, vollständige Unicode-Zeilenbruchregeln und echte assistive/
IME-/Geräte-/Leistungsabnahmen bleiben offen. Neue Plattform-/Paketabnahme folgt
in STATUS. Die vorherige 0.9.36-CI besteht inzwischen alle 20 Jobs/vier Pakete.

## Ergänzende Beschriftungsprüfung vom 8. Oktober

Ab 0.9.38 entfallen visuelle Messungen verdeckter Buttons; ihre Tastaturziele
bleiben erhalten. Label-/Hinweispuffer und Auslassungszeichen berücksichtigen
Grapheme. Lokale Gesamt-/Desktop-/Sanitizerprüfung besteht;
[Umfang und Grenzen](BESCHRIFTUNGEN.md). Das kalte Layout langer Dokumente ist
damit nicht abgeschlossen. Eine weitere Vorbereitung muss Abbruch, Datei-/
Schriftwechsel, Font-Threadbesitz und unveränderte Zeilen-/Scrollgeometrie erhalten.

## Schutzdialog vom 8. Oktober

Ab 0.9.39 trennt der [Schutzdialog](SCHUTZDIALOG.md) ungespeicherter Änderungen
einen scrollbaren Meldungskörper von festem Titel, Schließen und Entscheidungen.
Große Schrift, lange Konfliktmeldungen, Scrollgrenzen und Tab ohne Positionsreset
sind lokal geprüft. Die neue Plattform-/Paketabnahme und reale native Bedienung
bleiben gesondert; dieser Schritt schließt die vollständige Release-Liste nicht.

## Wortbedienung vom 8. Oktober

Ab 0.9.40 sind eigene Unicode-18-Defaultgrenzen mit lokalen Kern-/UI-/
Sanitizerprüfungen an Wortbewegung, Shift-Auswahl und Doppelklick angebunden.
Mac erhält Option-/Command-Pfeile. [Vertrag](WORTNAVIGATION.md). Native
Wort-/Zeichenrechtecke, sprachabhängige gemischte Schreibrichtungen, Wörterbuch-
Segmentierung und tatsächliche assistive/IME-/Geräteabnahme bleiben getrennt.
Die untersuchte Zeilenwiederverwendung beschleunigte den kalten langen Wechsel
nicht und wurde entfernt; dieser Leistungsschritt bleibt offen.
