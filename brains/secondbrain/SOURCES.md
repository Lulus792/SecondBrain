# SecondBrain Quellen

Zuletzt eingesehen: 11. Oktober 2026. Relative Verweise erwarten diese Instanz
unter brains/secondbrain im Repository. Quellen haben unterschiedliche Aufgaben.

## Auftrag und Arbeitsweise

- [Projektplan](../../docs/PROJEKTPLAN.md): verbindliche Nutzeranforderungen und Todos.
- [Projektanweisungen](../../AGENTS.md): C, UI-Abhängigkeiten, GitHub und Arbeitsregeln.
- [Grundlagen](../../docs/GRUNDLAGEN.md): Second-Brain-Konzept mit Originalquellen.
- [Datenvertrag](../../docs/DATENVERTRAG.md): Metadatenversionen, Textgrenzen, Altformat und Updateverhalten.
- [Konzept](../../docs/KONZEPT.md): Wissensstruktur und gemeinsame Pflege.

## Gestaltung und Architektur

- [UI-Recherche](../../docs/UI_RECHERCHE.md): Apple-Originalquellen und Designprinzipien.
- [UI-Entwurf](../../docs/UI_ENTWURF.md): Ansichten, Abläufe und Gestaltung.
- [Wissensgalaxie](../../docs/UI_GALAXIE.md): neue Referenzen, bestätigte
  Designvorlieben, drei Entwurfsreferenzen und die ausgewählte C-Integration.
- [Tastatur und räumliche Oberfläche](../../docs/UI_TASTATUR.md): Bedienvertrag,
  Apple-Grundlagen und Graphumfang.
- [Architektur](../../docs/ARCHITEKTUR.md): C17-Kern, Plattformschicht, UI und Build.
- [UI-Politur vor 1.0](../../docs/UI_POLITUR.md): neuer Auftrag, erneute Apple-Recherche und Abnahmekriterien.
- [Sicherung und Wiederherstellung](../../docs/SICHERUNG.md): eigener C-Vertrag,
  Bestandsschutz, Fehlerprüfungen und geplanter Desktop-Ablauf.
- [Erster Start](../../docs/ERSTER_START.md): Apple-Grundlage, direkte Projektwege und Prüfgrenzen.
- [Einstellungen und Arbeitsstand](../../docs/EINSTELLUNGEN.md): Dateivertrag,
  native Ordnerwahl, Wiederherstellung und Prüfgrenzen.
- [Native UI-Anbindung](../../docs/BARRIEREFREIHEIT_PLAN.md): integrierte Adapter, tatsächliche macOS-Provider-/UIA-/AT-SPI-Clientprüfungen und offene assistive Abnahme.
- [Gemeinsame Inline-/Linkregeln](../../docs/INLINE_LINKS.md): echte Aktionen, Sternkartenbeziehungen und Grenzen.
- [Markdown-Blockregeln](../../docs/MARKDOWN.md): gemeinsame C-Erkennung, Originalbytes und verbleibender Umfang.
- [Tabellen](../../docs/TABELLEN.md): eigene C-Regeln, adaptive Leseansicht und native Schnittstellengrenzen.
- [Dokumentstruktur](../../docs/DOKUMENTSTRUKTUR.md): native Blockstruktur, Überschriften und Abschnittssprünge.
- [Emoji und Schriftwahl](../../docs/EMOJI.md): Quelle, Lizenz und Mess-/Rasterabnahme.
- [Grapheme](../../docs/GRAPHEME.md): eigener C-Algorithmus, Zeicheneinheiten, Unicode-Daten und Prüfgrenzen.
- [IME-Komposition](../../docs/IME.md): Originalquellen, vorläufige Anzeige, Eingabeordnung und Prüfgrenzen.
- [Textdarstellung](../../docs/TEXTDARSTELLUNG.md): UI-Schriftrollen, geformte Textläufe, Fallback und verbleibende Textarbeit.
- [Bidi-Grundlage](../../docs/BIDI.md): eigener UTF-8-Absatzplan, Unicode-18-
  Daten, gefundene Bibliotheks-/Generatorfehler, lokale Abnahme und verbleibende
  gemeinsame Renderer-/Editor-/native Anbindung.
- [Geformte Glyphenpositionen](../../docs/GLYPHENGEOMETRIE.md): gemeinsamer
  C-Zeilenplan, kontextuelle Stil-/Schriftteile, Unicode-18-Spiegelanbindung,
  Raster-/Sanitizer-Nachweise, Reader-Einbindung und offene Editor-/native Anbindung.
- [Lizenzansicht](../../docs/LIZENZEN.md): Originaltexte in der App, Bedienwege und Prüfgrenzen.
- [MIT-Lizenz](../../LICENSE): gewählte Lizenz des eigenen Codes.
- [UI-Abhängigkeiten](../../third_party/README.md): Herkunft, Versionen, Lizenzen, Anpassungen.

## Tatsächlicher Stand und Nachweise

- [Umsetzungsstand](../../docs/STATUS.md): datierte Implementierung, Prüfungen und Grenzen.
- [Plattformnachweise](../../docs/PLATTFORMEN.md): erfolgreiche Läufe mit konkretem Umfang.
- [GitHub Actions](https://github.com/Lulus792/SecondBrain/actions): Status konkreter Commits.
- [CI-Konfiguration](../../.github/workflows/tests.yml): vorgesehene Prüfungen; kein Erfolg allein.
- [Kernprüfung](../../tests/test_core.c), [Zustandsprüfung](../../tests/test_model.c),
  [Editorprüfung](../../tests/test_ui.c), [Bedienprüfung](../../app/self_test.c):
  Prüfumfänge; ihr Quelltext allein belegt keine ausgeführten Ergebnisse.
- [Paketprüfung](../../tests/test_package.cmake): Start und Versionsidentität der entpackten Anwendung.
- [Versionsprüfung](../../tests/test_version.cmake): tatsächliche Prozessausgabe ohne UI-Start.
- [Vorabversion v0.7.3](https://github.com/Lulus792/SecondBrain/releases/tag/v0.7.3): vier veröffentlichte Pakete und Prüfsummen.

## Anwendung verwenden

- [Anleitung](../../README.md): Build, Start und Arbeitsweise.
- [README-Recherche](../../docs/README_RECHERCHE.md): neun GitHub-Originale und die abgeleitete Struktur.
- [Release-Aufgaben](../../docs/RELEASE.md): bewertete Lücken vor 1.0 und optionale Erweiterungen.
- [Distribution](../../docs/DISTRIBUTION.md): Pakete, Voraussetzungen und Einschränkungen.
- [Kernquellen](../../src) und [App-Quellen](../../app): konkrete Implementierung.

## Verdichtetes Wissen

- [Anforderungen und Abnahme](knowledge/anforderungen.md).
- [Pflege des gemeinsamen Gedächtnisses](knowledge/pflege.md).

## Support und Mitarbeit

- [Support](../../SUPPORT.md): öffentliche Meldungen, benötigte Angaben und Grenzen.
- [Beiträge](../../CONTRIBUTING.md): Architekturvorgaben und Prüfwege für Änderungen.
- [Abhängigkeitswartung](../../docs/ABHAENGIGKEITEN_WARTUNG.md): feste Quellen und Updateablauf.

- [Abschnittstrennungen](../../docs/TRENNLINIEN.md): eigene Blockregel, Gestaltung, native Semantik und Prüfgrenzen.

- [Inline-Stile](../../docs/INLINE_STILE.md): gemeinsamer C-Leser, Unicode-Zeichengruppen, Darstellung und Grenzen.

- [Native Textstile](../../docs/NATIVE_TEXTSTILE.md): Stilbereiche, Fontattribute, echte native Prüfungen und Geometriegrenzen.

- [Windows-UIA-Textsuche](../../docs/UIA_TEXTSUCHE.md): bestätigter Adapterbefund, feste Quellkorrektur und native Prüfgrenzen.

- [Zeichenreferenzen-Recherche](../../docs/ENTITIES_RECHERCHE.md): Grundlagen, Originaldaten, Lizenz und Ausgabegrenzen; die Umsetzung steht im folgenden Vertrag.

- [Zeichenreferenzen](../../docs/ENTITIES.md): eigene C-Verarbeitung, Ausgabegrenzen, echte UI-/Graphprüfungen und Daten-/Fontlizenzen.

- [Autolinks](../../docs/AUTOLINKS.md): gemeinsame Adressregeln, URI-Übergabe und Prüfgrenzen.
- [Referenzlink-Recherche](../../docs/REFERENZLINKS_RECHERCHE.md): Originalgrundlagen vor der dokumentweiten Definitions-/Unicode-Implementierung.

- [Referenzlinks](../../docs/REFERENZLINKS.md): dokumentweite C-Umgebung, Unicode-Faltung, Quellen und verbleibende Container-Grenzen.

- [Containerplan](../../docs/CONTAINER_PLAN.md): Originalregeln, implementierte Quellprojektion und noch ausstehende App-Integration.

- [Dokumentbaum](../../docs/DOKUMENTBAUM.md): implementierte Container-/Blattstruktur, projektionsgebundene Referenzen, Prüfungen und ausstehende gemeinsame App-Anbindung.

- [Listen und Zitate](../../docs/CONTAINER_UI.md): vor Umsetzung erneut gelesene
  Apple-Grundlagen, gemeinsame Baum-Anbindung, Darstellung und native Grenzen.

- [Reaktionszeit und Flächen](../../docs/INTERAKTION.md): gemeinsame Zeitplanung,
  Kartenwechsel, Scrollen, Dialogmessung, Hinweise und Suchfläche ab 0.9.18.

- [Letzter gültiger Sternkartenstand](../../docs/STERNKARTEN_BESTAND.md): zusammenhängende Desktop-Inventur, stabile Kennungen, Fehler- und Wiederherstellungsvertrag ab 0.9.19.

- [Navigation und Texteingabe](../../docs/NAVIGATION_POLITUR.md): Mess-/Umbruchplancaches, Startfokus, Suchende, Textcursor und Anlegekarten ab 0.9.19.

- [Lizenzinventur](../../docs/LIZENZ_INVENTUR.md) und [Manifest](../../third_party/license-manifest.json): ausgewertete Zielplattform-Abhängigkeiten, Originale, Hashes und offene Laufzeitanteile ab 0.9.20.

- [CI-Teststarter](../../tools/test_entry.py) und [Prozessprüfung](../../tests/test_ci_entry.py): frühes, natives Eintragslog; keine unbelegte Fehlerursache.

- [Rust-Laufzeitnachweis](../../docs/RUST_RUNTIME_NACHWEIS.md): originale Library-Lockdatei und buildbezogene Compileridentität; zusätzliche Original-Lizenzen bleiben offen.

- [Paket-Laufzeitprüfung](../../docs/PAKET_LAUFZEIT.md): native Imports/Abhängigkeitsauflösung, Systemverträge, tatsächliche Fixture-/Paketnachweise und Grenzen.

- [Weitere Navigationspolitur](../../docs/NAVIGATION_POLITUR.md): GPU-Snapshot,
  freie Suchflächen, Anfangsgeometrie und erneute Apple-Originale am 8. Oktober.
- [Tabellenmatrix](../../docs/TABELLEN.md): native Mac-Matrix, verbleibende
  Windows-/Linux-Anbindung und getrennte Screenreader-Abnahme.

- [Cursorgeometrie](../../docs/CURSORGEOMETRIE.md): tatsächliche Glyphen-/GDEF-
  Positionen, Bidi-Affinität, Maus-/Auswahlregeln, Indizes und offene Editor-/
  IME-/native Integration.

- [Produktive Editor-/Feldgeometrie](../../docs/EDITORGEOMETRIE.md): gemeinsamer
  Plan für Cursor/Auswahl/Maus/Pfeile/IME, Backing-Pixelclips, Cache und
  verbleibende native/Unicode-/Leistungsabnahme.

- [Einfache UI-Textgeometrie](../../docs/PLAIN_TEXT.md): gemeinsamer Absatz-/
  Script-/Glyphenplan, graphemgebundene Kürzung, exakter Cache und verbleibender
  allgemeiner Widgetumbruch ab 0.9.36.

- [Gemeinsamer allgemeiner Umbruch](../../docs/WRAPPED_TEXT.md): Absatzkontext,
  tatsächliche Höhen, Widgetclips, Abschluss-/Cachepolitik und unabhängige
  Referenzprüfung ab 0.9.37.

- [Beschriftungen und verdeckte Buttons](../../docs/BESCHRIFTUNGEN.md):
  graphemgebundene Kürzung/Puffer, erhaltene Tastaturziele, Hoverclips und
  verbleibendes kaltes Dokumentlayout ab 0.9.38.

- [Schutzdialog](../../docs/SCHUTZDIALOG.md): feste Kopf-/Aktionsfläche, eigener
  Meldungsscrollweg und sicherer Fokus-/Abbruchvertrag ab 0.9.39.

- [Wortnavigation](../../docs/WORTNAVIGATION.md): eigene Unicode-Defaultgrenzen,
  UI-Wortauswahl, Plattformtasten, Originaldaten und gesonderte native Grenzen ab40.

- [Native Textläufe](../../docs/NATIVE_TEXTLAEUFE.md): begrenzte Graphem-/
  Wortmetadaten, Quellenpositionen, Cache-/Abbruchvertrag und weitere Geometriearbeit.

- [Native Editorrechtecke](../../docs/NATIVE_TEXTGEOMETRIE.md): tatsächlicher
  Glyphen-/Cursorplan, Scrollen, Bidi, native Retina-Koordinaten und Grenzen ab42.

- [Reader-Vorbereitung](../../docs/READER_VORBEREITUNG.md): negativer
  Breitenmessvergleich, erneute Apple-Grundlagen und noch nicht integrierter
  Zustands-/Eigentums-/Abnahmevertrag für den langen Wechsel.

- [Font-Snapshots](../../docs/FONT_SNAPSHOTS.md): unveränderliche Ressourcen,
  private Worker-Fonts, geprüfte Quell-/Fontbindung und Grenzen des Prototyps.

## Reader-Anbindung vom 11. Oktober

- [Vorbereitungsvertrag](../../docs/READER_VORBEREITUNG.md): Zustände, produktive Anbindung und offene Leistungsgrenzen.
- [Schriftbesitz](../../docs/FONT_SNAPSHOTS.md): private CPU-Kontexte, Batch-Budgets und Freigabe.
- [Produktiver Prüfer](../../tests/test_reader_prepare.c): Umbruch-/Tabellen-/Scrollvergleich, Resize, Retarget, Schriftwechsel, modaler Reader und CRLF-Quellen.
- [Batch-Prüfer](../../tests/test_prepare.c): genaue Glyphenbindung, über 2048 gehaltene Pläne, Budgetabweisung und Abbruch.

- [Ebenen-/Fokusvertrag](../../docs/UI_EBENEN.md): transparente Vordergrundflächen,
  sichtbare Tabziele und akzeptierte Notizwechsel, erneut geprüfte HIG-Quellen.
- [Ebenenprüfer](../../tests/test_map_layers.c): tatsächliche Karten-/Listen-/
  Werkzeugpixel, sichtbarer Fokus, Größen/Kontrast, Suchrückkehr und F6.

- [Rasterleistung](../../docs/RASTER_LEISTUNG.md): bitgleiche Glasinnenflächen,
  eingefrorene Versionsreferenz und alternierende produktive Metal-Messung.
- [Rasterprüfer](../../tests/test_space_raster.c): vollständige RGBA-Daten,
  Modi, Cache/Resize, Randfälle und64 Flächen gegen die0.9.47-Referenz.

- [Native Eingabefeldgeometrie](../../docs/NATIVE_TEXTGEOMETRIE.md): sofortige
  Quell-/Planübernahme, IME, Scrollversatz, Eigentum und erhaltener Editorcache ab49.
- [Produktiver Feldprüfer](../../tests/test_field_geometry.c): alle sieben
  Feldtypen, Bidi/Grapheme, große Schrift, Scrollen und IME.

- [Native Feldaktionen](../../docs/NATIVE_FELDAKTIONEN.md): Auswahl-/Ersetzungs-
  Vertrag, Quellen-/Kontextschutz und eigene Mac-Setter-/Besitzeranbindung ab50.
- [Aktionsprüfer](../../tests/test_field_actions.c): produktive Queue und Widgets,
  Graphemgrenzen, Kapazität, Fokusrückkehr und geschlossene/geänderte Felder.

- [Native Leseblöcke](../../docs/NATIVE_LESEGEOMETRIE.md): gezeichnete
  Graphemflächen, gehaltene Metadaten/Worker-Budgets und Metal-Vergleich ab51.
- [Readerprüfer](../../tests/test_reader_native.c): unabhängige Glyphen-/
  Ausrichtungs-/Ursprungsprüfung und produktive Absatz-/Code-/Tabellenquellen.

- [Native Titel und Markierungen](../../docs/NATIVE_TITELGEOMETRIE.md):
  tatsächliche Textbefehle, sichtbare Kürzung, eigene Geometrieausgaben und Cachebudget.
- [Titelprüfer](../../tests/test_title_geometry.c): unabhängiger Glyphenvergleich,
  Schriftgrößen, lange Namen, Marker, dekorative und modale Titel.

- [Reader-Quellplan](../../docs/READER_QUELLPLAN.md): eigener Dokumentbaum,
  exakte Quellbindung, Referenzen, Speicherbudget und serialisierte Metal-Messung.
- [Quellprüfer](../../tests/test_reader_source.c): Quellenbesitz, Änderungen
  gleicher Länge, Referenzumgebung und vollständiger Budget-Rückfall.

- [Icon-/Materialnachprüfung](/Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-11_15-36-09/UI_REVIEW.md): SBUI-076/078 lokal korrigiert,
  SBUI-077 offen; aktuelle HIG-Kapitel, 100/200%-Aufnahmen und Export-Referenz.
- [Icondefinitionen](../../tools/export_icons.py): eigene C-/SVG-Motive,
  vollständige Zuordnung und erhaltene Punktflächen beim Export.

- [Dialogmaterial](../../docs/DIALOGMATERIAL.md): aktuelle Texturauflösung,
  gerundete Ebenenkomposition, Softwaredreiecke und Aufnahme-/Abnahmegrenzen.
- [Materialprüfer](../../tests/test_material_panel.c): unabhängiger Vergleich
  von Hintergrundtext, Vordergrundtext und Außenrand in sechs Darstellungsmodi.
- [Frame-Messung](../../docs/FRAME_MESSUNG.md): Eingabe, erste Bilder, reale
  Tickabstände und CSV-Einzelwerte; serielle Metal-/Software-Nachweise und Grenzen.
- [Reader-Inline-Plan](../../docs/READER_INLINE_PLAN.md): gehaltene Hervorhebungen,
  Referenzen und Linkiteratoren im Quellbudget, Fehlerbereinigung und Metal-Vergleich.
- [Reader-Stilplan](../../docs/READER_STILPLAN.md): gehaltene Graphem-Stilgrenzen,
  eigene Ausgabekopien, zusätzliches Budgetlimit und serielle Metal-Messung.
- [Reader-Rückkehr](../../docs/READER_RUECKKEHR.md): stabile Inhaltsidentität unter
  gewöhnlichen Modalaufgaben, Scrollkörper-/Titelregeln und offener Formularbefund.
- [Hilfe-Rückkehr](../../docs/HILFE_RUECKKEHR.md): Ursprungsaufgabe, Feldauswahl,
  Scrollposition und modale Befehlsgrenzen ab 0.9.59; lokale Prüfgrenzen.
- [Produktiver Hilfeprüfer](../../tests/test_help_return.c): SDL-Ereignisse,
  native Auswahl/Ersetzung, gescrollte Einstellungen, Kontextreader und Hilfe-Schaltfläche.
- [SBUI-077-Nachprüfung](/Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-11_16-38-09/UI_REVIEW.md): Software-/Metalbilder und offene native
  Snapshot-, Plattform-, Geräte- und assistive Abnahmen.
