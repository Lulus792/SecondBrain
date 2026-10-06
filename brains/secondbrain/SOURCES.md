# SecondBrain Quellen

Zuletzt eingesehen: 7. Oktober 2026. Relative Verweise erwarten diese Instanz
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
- [Dokumentstruktur](../../docs/DOKUMENTSTRUKTUR.md): native Blockstruktur, Überschriften und Abschnittssprünge.
- [Textdarstellung](../../docs/TEXTDARSTELLUNG.md): UI-Schriftrollen, geformte Textläufe, Fallback und verbleibende Textarbeit.
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
