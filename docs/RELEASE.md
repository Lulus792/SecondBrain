# Auf dem Weg zu 1.0

Stand: 11. Oktober 2026. Diese Liste bewertet den vorhandenen Code und die
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
  Die 0.9.59-Paketprüfung bestätigte außerdem eine Timing-Grenze: Ein Scrollbefehl
  während der Lizenzreader-Vorbereitung kann vor der fertigen Ansicht verpuffen.
  Der Tastaturtest wartet für seine Prüfung der fertigen Leseansicht begrenzt
  auf deren Bereitschaft. Ab 0.9.61 bleiben diese Eingaben bis zur gemessenen
  fertigen Ansicht erhalten; [lokale Korrektur und Prüfgrenzen](SCROLLEN_BEIM_LADEN.md). Ab0.9.63
  entspricht die native Ausgabe dem sichtbaren Ladezustand; vorzeitige vollständige
  Markdown-Textläufe entfallen. [Messung und Grenzen](LADEZUSTAND_NATIVE.md).
- [ ] **Dauerhafte Release-Pakete veröffentlichen:** versionierte GitHub Releases,
  Prüfsummen, Änderungsübersicht und reproduzierbare Paketabnahme. Actions-Artefakte
  verfallen nach 30 Tagen. Signierung, macOS-Notarisierung und einfache Installation
  müssen für die gewählten Vertriebswege geklärt werden. Ein automatischer Updater
  ist optional, ein dokumentierter Updateweg notwendig.
  Der manuelle Paketwechsel63→64→63 ist inzwischen auf Intel/macOS mit erhaltenen
  Originalbytes, Einstellungen und Sicherung/Wiederherstellung geprüft;
  [Umfang und weitere Plattformabnahmen](UPDATE_NACHWEIS.md).
  Die dauerhafte Vorabversion [v0.7.3](https://github.com/Lulus792/SecondBrain/releases/tag/v0.7.3) ist am
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

- [x] **Fokus nur in der aktiven Modalaufgabe zeichnen (lokal):** Die Kontextaufnahme
  zur Scrollkorrektur zeigt denselben Reader-Fokusrahmen im abgeschirmten
  Formularhintergrund. Native Ziele sind dort gesperrt; die visuelle
  Ringzeichnung benötigt eine eigene Nachprüfung. Original:
  `build/reader-scroll-verification/context-ready.png`. Ab 0.9.62 bindet die
  Ringzeichnung den Fokus an die aktive Modalgruppe; 310 gezielte Bedingungen
  prüfen Vordergrund, Hintergrund und Escape-Rückkehr. [Nachweis und Grenzen](MODALER_FOKUS.md).

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

## Aktuelle Plattformbefunde vom 11. Oktober

Die Jobs zum Quellstand 0.9.58 wurden erneut eingesehen:
[CI-Lauf 38158192485](https://github.com/Lulus792/SecondBrain/actions/runs/38158192485).
Ein zuvor beobachteter Wartestatus ist kein aktueller Erfolgsnachweis.

- [x] Windows-UI-Build korrigieren: MSVC meldete in `app/plain_native.inc:35`
  C2036 (`void *`: unbekannte Größe). Die Cache-Inventur muss vor der
  Indexberechnung einen typisierten Zeiger verwenden. Release-Job 114525404584;
  Debug-Job 114525404631 ebenfalls fehlgeschlagen.
  Ab 0.9.60 typisiert korrigiert und lokal mit strenger C17-Prüfung/Cache-
  Belastung/Sanitizern geprüft. Im Lauf 38162552488 zu fe7d18e bestehen die
  tatsächlichen MSVC-UI-Builds in Debug (114538327441) und Release
  (114538327369). UI-/Paketprüfungen liefen bei dieser Abfrage noch.
- [x] Linux-Resize-Prüfung untersuchen: `prepared-document-reader` scheiterte
  an `tests/test_reader_prepare.c:54`, unmittelbar nach Fenstergrößenänderung
  bei `loading(&d)`. Ein bestandener Build ersetzt diese Laufzeitprüfung nicht.
  Ab 0.9.60 synchronisiert der Prüfer die zulässige tatsächliche Größe;
  die lokale Nachprüfung besteht, native Linux-Nachprüfung folgt.
  Im neuen Release-Lauf 38162552488 besteht der Reader-Resize-Test tatsächlich;
  der Linux-Debug-/Release-Gesamtlauf scheitert noch an der getrennten Stilprüfung.
- [x] Linux-native Textstilabfrage untersuchen: `native-accessibility` scheiterte
  bei AT-SPI-Operation 11 auf „Stilprüfung“ (`object/pattern not found`).
  Release-Job 114525404659 und Debug-Job 114525404463 haben fehlgeschlagene
  UI-Prüfungen; die Paketprüfungen wurden dadurch nicht ausgeführt.
  Der Prüfer sucht ab 0.9.60 für Textstile einen namensgleichen Dokument-/
  Eingabebereich mit angefordertem Inhalt. Die native Linux-Nachprüfung besteht
  zu b6490bd in Debug und Release; erwartete Stilwerte bleiben unverändert.

Windows-/Linux-Kernprüfungen bestehen in diesem Lauf; die macOS-Jobs wurden
zuletzt weiterhin als queued beobachtet. Die UI-Befunde benötigen Korrekturen
und neue native Nachweise. Logauszüge: `build/help-return-verification/prior-*-failure.log`.

Zu b6490bd/0.9.60 sind alle vier Windows-/Linux-UI-Jobs einschließlich nativer
Prüfungen erfolgreich; beide Release-Jobs bestehen auch die entpackten Pakete.
[Lauf 38164624696](https://github.com/Lulus792/SecondBrain/actions/runs/38164624696).
Der Absatz davor beschreibt den früheren 0.9.58-Lauf. Native macOS-Jobs zuletzt
queued; lokale Intel/macOS-Abnahme separat. Keine menschliche Screenreader-Abnahme.

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

## Native Textläufe vom 8. Oktober

Ab0.9.41 sind native Textläufe auf255 Auswahleinheiten begrenzt und erhalten
Unicode-Wortanfänge mit ganzem Quellenkontext. Normale Grapheme und CRLF
werden als Einheiten publiziert. Auswahlen über mehrere Läufe werden unter
Sperre auf globale Editorpositionen abgebildet; veraltete Eingabezuordnungen
werden verworfen. [Vertrag und genaue Grenzen](NATIVE_TEXTLAEUFE.md).
Eine lokale native Mac-Auswahl über Laufgrenzen ist nachgeprüft.
Glyphenrechtecke, sehr lange native Grapheme und reale assistive/
sprachabhängige Abnahmen bleiben weitere offene Textaufgaben.

## Native Editorrechtecke vom 8. Oktober

Ab 0.9.42 übernimmt der Notizeditor die tatsächliche Zeichen-/Zeilengeometrie
einschließlich Bidi und Scrollposition. Die native Fenstertransformation
korrigiert den auf dem Intel/Retina-Mac bestätigten Skalierungsfehler.
[Vertrag und Prüfgrenzen](NATIVE_TEXTGEOMETRIE.md). Weitere Felder und die
Leseansicht, Wiederverwendung unveränderter Geometriemetadaten, neue native
Zielplattformprüfungen und reale assistive/IME-/HiDPI-Abnahmen bleiben offen.

## Wiederverwendung nativer Editorpläne vom 8. Oktober

Ab 0.9.43 werden native Editor-Metadaten bei unverändertem Quellen-/Schrift-/
Zeilenplan mit einem eigenen 32-MiB-Budget wiederverwendet. Scroll- und
Cursoränderungen benötigen keine neue Unicode-/Wortaufbereitung. Ausgaben
behalten eigenes Eigentum; übergroße Exporte bleiben vollständig ohne
zusätzliche dauerhafte Kopie. [Vertrag](NATIVE_TEXTGEOMETRIE.md).
Dies schließt weder das erste lange Layout noch die Integration weiterer
Felder/Leseblöcke oder reale Geräte-/Eingabe-/Screenreader-Abnahmen ab.

## Reader-Vorbereitung vom 11. Oktober

Ab 0.9.46 bereitet der produktive Reader lange Dokumente blockweise in einem
privaten CPU-Fontkontext vor. Die UI verwendet fertige, gehaltene Pläne für
Umbruch, Höhen, Geometrie und Zeichnung; der allgemeine Memo-Cache verdrängt
sie nicht. Retargeting wartet nicht auf einen laufenden Worker. Resize erhält
die logische Scrollposition; Quellenrückkehr erhält den früheren Linkfokus nach
der Vorbereitung. Aktuelle Hinweise unterbrechen überlagernde Überblendungen.
[Vertrag und tatsächliche Grenzen](READER_VORBEREITUNG.md).

Die vollständige Leistungsaufgabe bleibt offen: Quelllesen, Parser/Inline-
Aufbereitung, native Metadaten und Rasterung bleiben UI-Arbeit. Budget-Rückfälle
sind synchron; Spitzenverbrauch einzelner riesiger Absätze, Shutdown und echte
Geräte/GPU/Monitore brauchen weitere Abnahmen. Die vorhandene Quellen-/Kontext-
und Blockgeometrieprüfung ersetzt keine Windows-/Linux- oder menschliche Prüfung.

## Native Feldgeometrie vom 11. Oktober

Ab 0.9.49 übernehmen Suche und alle Formulartexte den gezeichneten Quellplan.
Die Ausgaben bleiben pro Feld getrennt und verdrängen den Notizeditorcache nicht.
[Vertrag](NATIVE_TEXTGEOMETRIE.md). Leseansicht, native Auswahl-/Ersetzungsaktionen
einzeiliger Felder und reale assistive/IME-/Geräteabnahmen bleiben offen.

## Native Feldaktionen vom 11. Oktober

Ab0.9.50 sind Auswahl und Ersetzung an alle einzeiligen Felder angeschlossen,
mit graphemgebundenen Grenzen, atomischer Kapazitätsprüfung und geschützter
Fokusrückkehr. Die eigene Mac-Anbindung ergänzt den im festgelegten Provider
fehlenden Setter für ausgewählten Text, auch im Notizeditor. [Vertrag](NATIVE_FELDAKTIONEN.md).
Neue Zielplattform-, IME-/Screenreader-/Geräteabnahmen und native Leseblock-Geometrie
bleiben offen.

## Native Leseblöcke vom 11. Oktober

Ab0.9.51 halten die Glyphenpläne Rechtecke und native Läufe für Absätze,
Code und gezeichnete Tabellenzellen. Die Hintergrundvorbereitung erzeugt diese
mit; Quelle, Zeilen, Bidi, Stile und Ausrichtung bleiben an der Zeichnung.
[Vertrag/Messung](NATIVE_LESEGEOMETRIE.md). Separate Kartentitel, Listenmarkierungen,
logische Kopfzellen ohne Widget, neue Zielplattformen und tatsächliche assistive/
IME-/Geräte-/Langzeitabnahmen bleiben offen.

## Native Titel und Markierungen vom 11. Oktober

Ab 0.9.52 übernehmen Titel, vorhandene native Beschriftungen und Listenmarker
ihre tatsächlich gezeichnete Geometrie. Gekürzter Text und vollständiger Name
bleiben getrennt; dekorative Titel duplizieren keinen Dokumentinhalt.
[Vertrag und Grenzen](NATIVE_TITELGEOMETRIE.md). Lokale Gesamt- und gezielte
Sanitizerprüfungen bestehen. Neue Plattform-/Paketabnahmen, tatsächliche
assistive Bedienung und die übrige Release-Liste bleiben offen.

## Wiederverwendung des Dokumentbaums vom 11. Oktober

Ab 0.9.53 hält der Vordergrundreader seinen Quellbaum und Referenzen bei
unveränderten Textbytes mit einem eigenen 16-MiB-Budget. Größen-, Schrift- und
Scrollwechsel benötigen keinen neuen Parserlauf. Lokale Gesamt- und gezielte
Sanitizerprüfungen bestehen; der [Vertrag](READER_QUELLPLAN.md) benennt die
Metal-Messung und den vollständigen synchronen Rückfall. Erste Aufbereitung,
Inline-Parsing, Rasterung, reale Geräte und die aktuellen Zielplattformen bleiben
weitere Leistungs- und Release-Arbeit.

## Erneute Icon-/Materialprüfung vom 11. Oktober

Ab 0.9.54 sind das Reloadmotiv und die gemeinsame Sicherungs-Iconspalte
nachgeprüft; der Export erhält die vorhandenen Anschlüsse. Die neue Bildprüfung
hält SBUI-077 offen: Beschriftungen hinter Dialogen passend beruhigen, während
der gewünschte Glascharakter erhalten bleibt. Aktuelle Paket-/Zielplattform-,
reale Bedien- und weitere Leistungsabnahmen bleiben getrennte Release-Arbeit.

## Dialogmaterial-Nachprüfung vom 11. Oktober

Ab 0.9.55 ist SBUI-077 im lokalen Pixel-/Darstellungs-/Ereignisumfang nachgeprüft:
Dialoge setzen ihre aktuelle Glastextur vor eigenem Inhalt wieder ein, ohne die
Beschriftungen anderer UI-Fenster durchscheinen zu lassen. Material- und
Gesamtprüfung sowie gezielte ASan/UBSan bestehen. [Vertrag](DIALOGMATERIAL.md).
Vier korrekt vor Present erfasste Metalbilder wurden angesehen; native Snapshot-
Pixelidentität und weitere Geräte-/Plattform-/assistive Abnahmen bleiben getrennt
offen. Die vollständige Leistungs- und Release-Aufgabe bleibt bestehen.

## Reaktionszeitmessung vom 11. Oktober

Der [Diagnoseprüfer](FRAME_MESSUNG.md) berücksichtigt jetzt Eingabeverarbeitung,
erste Bilder und tatsächliche Animationsabstände. Serielle Metal-/Software-
Einzelwerte bestätigen verbleibende Anfangs- und Bewegungsarbeit. Das ist eine
verbesserte Messgrundlage, keine abgeschlossene Leistungskorrektur. Erste Quell-/
Inline-/native Aufbereitung, reale Eingaben und weitere Zielgeräte bleiben offen.

## Wiederverwendung der Inline-Analyse vom 11. Oktober

Ab 0.9.56 hält der [Reader-Inline-Plan](READER_INLINE_PLAN.md) Hervorhebungen und
Verweise in seinem Quellenbudget. Lokale Gesamt-/Sanitizerprüfungen bestehen;
serielle Metal-Paare zeigen weniger Layoutarbeit. Die erste Vorbereitung,
Tabellenzellen, native Metadaten/Bildausgabe und vollständige Geräte-/Plattform-
und Bedienabnahmen bleiben offen. Dieser Schritt schließt SBUI-074 nicht ab.

## Wiederverwendung der Stilgrenzen vom 11. Oktober

Ab 0.9.57 hält der [Reader-Stilplan](READER_STILPLAN.md) graphemgebundene
Stilspannen unter dem gemeinsamen Quellenbudget. Lokale Gesamt-/Sanitizer-
prüfungen und zusätzliche Budgetausgabe bestehen. Serielle Metal-Paare zeigen
weniger Layoutarbeit; erste Vorbereitung, andere Textarten, native Ausgaben/
Bildausgabe und vollständige Geräte-/Plattform-/Bedienabnahme bleiben offen.

## Reader-Rückkehr und offener Formularbefund vom 11. Oktober

0.9.58 erhält die lange Notiz und ihre Leseposition bei gewöhnlichen Menü-/
Formwechseln; Quelle/Worker bleiben gültig und der feste Titel bleibt einmalig.
[Reader-Rückkehr](READER_RUECKKEHR.md) dokumentiert lokale Gesamt-/Sanitizer-/
Bildnachweise und die Grenzen. SBUI-079 lokal nachgeprüft, keine Geräte-/
Plattform-/assistive Gesamtabnahme.

- [x] SBUI-080 im lokalen Ereignis-/Widget-/Providerumfang: F1/Hilfe kehrt ab
  0.9.59 zur Ursprungsaufgabe zurück; Felder, gerichtete Auswahl, Fokus,
  Fehlermeldung und Scrollposition bleiben erhalten. Umschalt+F10 verdrängt
  keine Modalaufgabe. [Vertrag und 92 gezielte Prüfbedingungen](HILFE_RUECKKEHR.md).
  Menschliche Bedienung und aktuelle weitere Zielplattformen bleiben offen.

## Ladezustand und aktuelle Windows-Debug-Nachprüfung

0.9.63 ist auf Intel/macOS lokal und im Paket geprüft. Die native Ausgabe folgt
dem sichtbaren Ladezustand; gemessene erste Bilddauer in drei Metal-Paaren etwa
20Prozent geringer. [Vertrag](LADEZUSTAND_NATIVE.md). Kein allgemeiner
Flüssigkeits- oder menschlicher Eingabenachweis.

- [x] Windows-Debug-Hilfeprüfung erneut nativ abnehmen: Im Lauf38169362014 zu
  22a8504 war nach zwei Bildern der Kontextreader noch nicht bereit. Der Prüfer
  wartet jetzt begrenzt auf die tatsächliche Bereitschaft; Höhen- und Rückkehr-
  assertions bleiben erhalten. Lokal und unter ASan/UBSan nachgeprüft.
  Windows-Release und beide Linux-Jobs einschließlich Release-Paketen bestehen
  für22a8504; sie schließen diesen Debug-Nachweis nicht ein. Mac-CI queued.

Hilfe-Nachprüfung zu798b84e: alle vier Windows-/Linux-Desktopjobs im
[Lauf38170374268](https://github.com/Lulus792/SecondBrain/actions/runs/38170374268)
erfolgreich, beide Release-Pakete geprüft; Mac-CI queued. Neue64-Fassung braucht
eigene Nachweise. Der [Mindestversionsvorschlag](PLATTFORM_VORSCHLAG.md) ist auf
Nutzerauftrag ausgearbeitet, noch keine endgültige Unterstützungszusage.

## Textvorbereitung vom 12. Oktober

Die [Optimierung 0.9.66](TEXT_VORBEREITUNG_OPTIMIERUNG.md) spart vollständige
Glyphengeometrie bei Umbruchversuchen und gemeinsame identische Batchpläne.
83 lokale Release-Tests und gezielte ASan/UBSan-Prüfungen bestehen; drei
Metal-Messpaare zeigen einen begrenzten Gewinn für das lange Journal.
SBUI-074, Geräte-/Trackpad-/assistive und weitere Plattformabnahmen bleiben offen.

## Materialfehlerprüfung vom 12. Oktober

- [x] Gezielte Raster-/Materialfehler lokal abnehmen: ab 0.9.67 bestehen sechs
  Fehlerstellen in 108 Kombinationen, Software und Metal, einschließlich
  lesbarem Vordergrund, bereinigten Teilpuffern und Rückkehr zur Glasdarstellung.
- [x] Normale Vordergrund-Materialpixel mit Metal prüfen: vor Present gelesene
  Kopie, 352 Bedingungen. Die produktive Übergangstextur ist weiter gesondert offen.

[Vertrag](MATERIAL_RUECKFALL.md). Gesamter Prozessspeichermangel/GPU-Geräteverlust,
aktuelle weitere Plattformen, reale Geräte und assistive Abnahmen bleiben offen.

## Übergangsbildprüfung vom 12. Oktober

- [x] Native Ausgangsbild-/Übergangstextur-Pixelidentität auf Intel/Metal in den
  geprüften Größen: 173 Interaktionsbedingungen, zwölf schnelle Wechsel und
  reale Größenwechsel. [Vertrag](UEBERGANG_BILD.md).
- [x] Alten Kartenzuschnitt bei Größenwechsel korrigieren: Aufnahmegröße halten,
  alte Überblendung vor neuem Zeichnen beenden, Kamerafahrt erhalten.

Echte Monitor-/Pixeldichtewechsel, weitere Geräte/Plattformen und menschliche
Flüssigkeitsabnahme bleiben offen. Die lokale Prüfung schließt diese nicht ein.
