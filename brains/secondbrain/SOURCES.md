# SecondBrain Quellen

Zuletzt eingesehen: 6. Oktober 2026. Relative Verweise erwarten diese Instanz
unter brains/secondbrain im Repository. Quellen haben unterschiedliche Aufgaben.

## Auftrag und Arbeitsweise

- [Projektplan](../../docs/PROJEKTPLAN.md): verbindliche Nutzeranforderungen und Todos.
- [Projektanweisungen](../../AGENTS.md): C, UI-Abhängigkeiten, GitHub und Arbeitsregeln.
- [Grundlagen](../../docs/GRUNDLAGEN.md): Second-Brain-Konzept mit Originalquellen.
- [Konzept](../../docs/KONZEPT.md): Wissensstruktur und gemeinsame Pflege.

## Gestaltung und Architektur

- [UI-Recherche](../../docs/UI_RECHERCHE.md): Apple-Originalquellen und Designprinzipien.
- [UI-Entwurf](../../docs/UI_ENTWURF.md): Ansichten, Abläufe und Gestaltung.
- [Wissensgalaxie](../../docs/UI_GALAXIE.md): neue Referenzen, bestätigte
  Designvorlieben und drei vorgeschlagene Ansichten; noch keine C-Implementierung.
- [Architektur](../../docs/ARCHITEKTUR.md): C17-Kern, Plattformschicht, UI und Build.
- [UI-Abhängigkeiten](../../third_party/README.md): Herkunft, Versionen, Lizenzen, Anpassungen.

## Tatsächlicher Stand und Nachweise

- [Umsetzungsstand](../../docs/STATUS.md): datierte Implementierung, Prüfungen und Grenzen.
- [Plattformnachweise](../../docs/PLATTFORMEN.md): erfolgreiche Läufe mit konkretem Umfang.
- [GitHub Actions](https://github.com/Lulus792/SecondBrain/actions): Status konkreter Commits.
- [CI-Konfiguration](../../.github/workflows/tests.yml): vorgesehene Prüfungen; kein Erfolg allein.
- [Kernprüfung](../../tests/test_core.c), [Zustandsprüfung](../../tests/test_model.c),
  [Editorprüfung](../../tests/test_ui.c), [Bedienprüfung](../../app/self_test.c):
  Prüfumfänge; ihr Quelltext allein belegt keine ausgeführten Ergebnisse.
- [Paketprüfung](../../tests/test_package.cmake): Start der entpackten Anwendung.

## Anwendung verwenden

- [Anleitung](../../README.md): Build, Start und Arbeitsweise.
- [Distribution](../../docs/DISTRIBUTION.md): Pakete, Voraussetzungen und Einschränkungen.
- [Kernquellen](../../src) und [App-Quellen](../../app): konkrete Implementierung.
- [Physim-Instanz](../physim/START.md): erste Beispielwissensbasis; kein neuer Physim-Test.

## Verdichtetes Wissen

- [Anforderungen und Abnahme](knowledge/anforderungen.md).
- [Pflege des gemeinsamen Gedächtnisses](knowledge/pflege.md).
