# Startprüfung unter Wayland

Stand: 12. Oktober 2026. Der zusätzliche Prüfweg ist vorbereitet; seine native
Linux-Ausführung steht noch aus. Bisherige Linux-Abnahmen laufen unter X11.

## Prüfweg

`tools/run_wayland_runtime.py` startet Weston mit Headless-Backend und dessen
angekündigter Pixman-Option. Ein privater Laufzeitordner mit Modus0700 enthält
den Wayland-Socket. Die CI startet den Prüfer in einer privaten D-Bus-Sitzung.
Weston verwendet keine Nutzerkonfiguration; GSettings bleibt im Speicher.

Der App-Prozess erhält ausdrücklich `SDL_VIDEO_DRIVER=wayland`. DISPLAY und
ein geerbter Renderertreiber werden entfernt. Die App verwendet ihren normalen
Renderer. Der vorhandene Laufzeitprüfer verlangt zusätzlich den tatsächlich
gemeldeten Videotreiber wayland, vier Frames, ein sichtbares Fenster und eine
Modulliste. Ein X11-Ausweichweg kann damit keinen erfolgreichen Bericht liefern.
Die isolierten Projektdateien und bestehende Berichte bleiben erhalten.

Der Compositor muss einen echten Unix-Socket bereitstellen und während der
Prüfung laufen. Bei Erfolg, App-Fehler oder Bereitschaftszeitüberschreitung
wird sein Prozess beendet und abgewartet. Logs und begrenzte Metadaten werden
als Entwicklungsnachweise aufbewahrt; Umgebungsvariablen werden nicht ausgegeben.

In CI wird sowohl die gebaute App als auch das entpackte Release-Paket geprüft.
Für den zusätzlichen Paketweg ist `-DSB_TEST_WAYLAND=ON` vorgesehen. Die normale
Paketprüfung benötigt ohne diese Option weiterhin keinen Weston.

## Grundlagen

- [SDL: Videotreiber auswählen](https://wiki.libsdl.org/SDL3/SDL_HINT_VIDEO_DRIVER),
  gelesen am12.Oktober: SDL_VIDEO_DRIVER wird vor SDL-Initialisierung ausgewertet.
- [Weston ausführen](https://wayland.pages.freedesktop.org/weston/toc/running-weston.html),
  Dokumentation16.0.90 gelesen am12.Oktober: Headless-Backend für Tests, Pixman
  als CPU-Renderer und Wayland-Socket im XDG_RUNTIME_DIR. Die tatsächliche CI-
  Version wird separat aufgezeichnet; verfügbare Rendereroptionen werden anhand
  ihrer eigenen Hilfe gewählt. Die ältere13.0.0-Manualquelle war beim Abruf
  nicht erreichbar und wird nicht als gelesene Grundlage ausgegeben.

## Lokale Nachweise und Grenzen

82 Python-Tests bestehen mit einer plattformspezifischen Überspringung. Zwei
neue Verträge verlangen den tatsächlich gewünschten Backend und erhalten die
übrigen Sichtbarkeits-/Framebedingungen. Vier echte lokale Prozess-/Unix-Socket-
Prüffassungen bestätigen Bereitschaft und Beenden nach Erfolg, Fehler im Prüfablauf,
frühem Prozessende und Bereitschaftszeitüberschreitung. Diese Fassungen sind
keine Weston-/Wayland-Ausführung auf dem Mac.

Der erweiterte Prüfer besteht zusätzlich mit der installierten
0.9.74/eabf5b3ff114 auf Intel/macOS14.6.1: Metal/Cocoa, vier Frames,377 beobachtete
Module und13 erhaltene isolierte Projektdateien. Das bestätigt den vorhandenen
Mac-Prüfweg, keinen Linux-Backend. Originale:
`build/accesskit-map-native/wayland-contract-tests.log`,
`wayland-process-checks.json` und `cocoa-probe-contract/`.

Auch eine erfolgreiche native Headless-Prüfung würde keine reale GPU,
Monitor-/Pixeldichtewechsel, Eingabegeräte, native Dialoge oder menschliche
Orca-Bedienung abnehmen. Diese Aufgaben bleiben in der [Release-Abnahme](RELEASE_ABNAHME.md).
