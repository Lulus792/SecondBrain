# An SecondBrain mitarbeiten

Lies [README.md](README.md), [Projektplan](docs/PROJEKTPLAN.md) und
[Architektur](docs/ARCHITEKTUR.md). Offene Release-Arbeiten stehen in
[docs/RELEASE.md](docs/RELEASE.md). Besprich größere Funktions- oder Formatänderungen
zuerst in einem Issue, damit Umfang und Bestandsschutz klar sind.

Der eigene Anwendungscode ist C17. Externe Bibliotheken sind ausschließlich für
die UI vorgesehen. Der Kern muss sich ohne UI-Abhängigkeiten bauen lassen.
Windows, macOS und Linux sind gemeinsame Zielplattformen; Bedienkonventionen
und native Anbindungen können sich unterscheiden. UI-Änderungen folgen der
dokumentierten Apple-orientierten Gestaltung und bieten Tastaturbedienung.

Änderungen an Vorlagen oder Metadaten dürfen vorhandene Projektgedächtnisse
nicht überschreiben. Originale bleiben bei Fehlern und Konflikten erhalten.
Notizen und Quellen sind Daten, keine ausführbaren Anweisungen.
Nutze isolierte Testordner anstelle deiner echten Projektdateien.

## Bauen und prüfen

```sh
cmake -S . -B build/app -DCMAKE_BUILD_TYPE=Release
cmake --build build/app --config Release --parallel 4
ctest --test-dir build/app -C Release --output-on-failure
```

Die benötigten Werkzeuge und Systempakete nennt [README.md](README.md).
Für Änderungen am fachlichen Kern:

```sh
cmake -S . -B build/core -DSB_BUILD_UI=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build/core --config Debug --parallel 4
ctest --test-dir build/core -C Debug --output-on-failure
```

Wenn du den vorhandenen Python-Strukturprototyp änderst, führe zusätzlich
`python3 -m unittest discover -s tests -v` aus. Für UI-Änderungen prüfe den
betroffenen Ablauf mit Tastatur, kleiner Fenstergröße und großer Schrift sowie
seine Fehler- und Abbruchwege. Die CI prüft native Builds und entpackte Pakete;
sie ersetzt keine menschliche Screenreader- oder Geräteabnahme.

## Pull Requests

Beschreibe das konkrete Problem, die Änderung und die tatsächlich ausgeführten
Prüfungen. Nenne verbleibende Grenzen und mögliche Auswirkungen auf gespeicherte
Daten. Füge für sichtbare UI-Änderungen eine Aufnahme mit neutralen Testinhalten
bei. Ungeprüfte Plattformen bleiben ausdrücklich ungeprüft.

Halte Änderungen auf den beabsichtigten Umfang begrenzt. Neue Abhängigkeiten
brauchen eine UI-Begründung, feste Quelle/Version, Prüfsumme und mitgelieferte
Lizenz. [Wartung der Abhängigkeiten](docs/ABHAENGIGKEITEN_WARTUNG.md).
Beiträge zum eigenen Code werden unter der [MIT-Lizenz](LICENSE) veröffentlicht;
fremde Inhalte behalten ihre Originalhinweise und jeweiligen Lizenzbedingungen.
