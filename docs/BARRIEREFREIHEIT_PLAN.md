# Native UI-Anbindung vorbereiten

Stand: 6. Oktober 2026. Der aktive Auftrag umfasst native Screenreader-Anbindung.
Dieser Text dokumentiert die recherchierte Anbindung; er ist noch kein Nachweis
für eine implementierte oder abgenommene Screenreader-Oberfläche.

Die [AccessKit C-Bindings](https://github.com/AccessKit/accesskit-c) bieten Adapter
für macOS, Windows und Unix. Das [SDL-Beispiel](https://github.com/AccessKit/accesskit-c/blob/0.23.1/examples/sdl/hello_world.c)
zeigt Aktivierung, Fokus und Aktionen. Der geprüfte Release 0.23.1 enthält C-Header
und statische UI-Bibliotheken für die benötigten Architekturen. Der lokale Download
stimmt mit dem GitHub-Asset-Digest überein:

`35b7ca8a6f1e038b5da35e1e9e5a0adaed9bfcf21e1496d29598fbbadcc7043f`

Ein Einsatz wäre ausschließlich für die UI-Anbindung vorgesehen. Fachlicher Kern,
Speicherung, Suche und Sicherung bleiben eigene C-Implementierungen. Die mögliche
Bibliothek ist intern in Rust geschrieben; der Anwendungscode verwendet ihre
C-Schnittstelle. Upstream nennt MIT/Apache und BSD-Anteile. Alle nötigen Hinweise
und Autoren müssen mit einer tatsächlichen Integration übernommen werden.

## Umsetzung und Abnahme

- Stabile, beschriftete UI-Knoten für Fenster, Aktionen, Eingabefelder und Inhalte.
- Rollen, Werte, Fokus, Sichtbarkeit und Geometrie entsprechen der wirklichen App.
- Native Aktionen werden sicher auf dem UI-Thread ausgeführt; Quellen bleiben
  schreibgeschützt und Entwürfe durch den vorhandenen Schutzdialog gesichert.
- Aktivierung und Abmeldung berücksichtigen die Lebensdauer des Fensters.
- Native Plattformadapter und tatsächliche Abfragen prüfen; ein internes Baum-Dump
  allein belegt keine vollständige Screenreader-Nutzung.
- Systemvorgaben für reduzierte Bewegung und Kontrast sowie Schrift-Fallback
  zusätzlich anbinden und prüfen.

Die [native Ordnerwahl von SDL3](https://wiki.libsdl.org/SDL3/SDL_ShowOpenFolderDialog)
ist asynchron. Ihre Ergebnisse müssen auf den UI-Thread gelangen; Abbrechen,
Fehler und ein inzwischen geschlossener Dialog dürfen keine falsche Navigation
oder Datenänderung auslösen. Tests verwenden eigene temporäre Daten und verändern
keine persönlichen Einstellungen des laufenden Systems.
