# SecondBrain Quellen

Zuletzt eingesehen: 8. Oktober 2026. Relative Verweise erwarten diese Instanz
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
