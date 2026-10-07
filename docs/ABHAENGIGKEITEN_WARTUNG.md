# UI-Abhängigkeiten pflegen

Stand: 7. Oktober 2026. Der eigene C-Kern verwendet keine externen Bibliotheken.
Die festgelegten UI-Komponenten und Schriften stehen mit Quellen, Versionen,
Prüfsummen, Lizenzen und lokalen Anpassungen in
[third_party/README.md](../third_party/README.md).

## Zuständige Dateien

| Bereich | Festlegung und Anpassungen |
| --- | --- |
| SDL3 | CMakeLists.txt; Fenster, Eingabe, Rendering und Dialoge |
| Nuklear | third_party/ui; eigener Text-, Graphem- und Eingabehook |
| SDL_ttf, HarfBuzz, FreeType | cmake/Text.cmake; eigene C-Anbindung in app/text.c |
| AccessKit | cmake/AccessKit.cmake, AccessKitLinux.cmake und PrepareAccessKitLinux.cmake; native Provideranpassungen |
| Schriften | assets/fonts; Originalquellen und Lizenzen in third_party |
| Unicode-Eigenschaftsdaten | third_party/unicode; festgelegte Daten und eigene C-Tabellen für Grapheme |

## Ablauf einer Aktualisierung

1. Original-Releasehinweise, relevante Sicherheitsmeldungen und geänderte
   Lizenzbedingungen lesen. Den tatsächlich geprüften Stand und die Quellen notieren.
2. Eine konkrete Version oder einen Commit samt SHA-256 festlegen. Keine
   unkontrollierte Verfolgung eines Entwicklungsbranches im normalen Build.
3. Lokale Anpassungen mit dem neuen Original vergleichen und gezielt übertragen.
   Besonders betroffen sind vollständige Eingaben, Textkommandos, native Rollen,
   AT-SPI-Cache-Protokoll und plattformspezifische Provideraktionen.
4. Passende Kern- und UI-Prüfungen ausführen. Anschließend Debug/Release und
   tatsächlich entpackte Pakete auf den Zielplattformen prüfen. Neue Fehler
   anhand ihrer konkreten Ursache beheben; ein grüner Build ersetzt keine Bedienprüfung.
5. Originaltexte und Herkunft in App und Paket aktualisieren. Geprüfte Änderungen
   und verbleibende Grenzen im Umsetzungsstand dokumentieren.

Aktualisierungen erfolgen gezielt; ein automatischer Dienst mit zugesagtem
Überwachungsintervall ist noch nicht eingerichtet. Vor einer stabilen Veröffentlichung
bleiben die vollständige Zuordnung transitiver Bestandteile, regelmäßige
Verantwortlichkeit und der vertrauliche Sicherheitsmeldeweg zu klären.

## GitHub-Vorlagen

Die Fehler- und Vorschlagsformulare folgen der am 7. Oktober gelesenen
[GitHub-Syntax](https://docs.github.com/en/communities/using-templates-to-encourage-useful-issues-and-pull-requests/syntax-for-issue-forms).
Sie fragen nachvollziehbare Abläufe und Buildangaben ab. Öffentliche Meldungen
und vertrauliche Sicherheitsberichte sind getrennte Wege; eine YAML-Vorlage
aktiviert keinen privaten Meldekanal. [GitHub-Konfiguration](https://docs.github.com/en/code-security/how-tos/report-and-fix-vulnerabilities/configure-vulnerability-reporting/configure-for-a-repository).
