# Reaktionszeit vollständig messen

Stand: 11. Oktober 2026. Messwerkzeug für den Anwendungsstand 0.9.55;
keine neue Produktversion und noch keine abgeschlossene Leistungsabnahme.

Der bisherige Prüfer begann seine Zeitmessung erst nach der Ereignisverarbeitung
und verwarf die ersten acht Bilder jeder Phase. Außerdem erhielt die Animation
immer 1/60 Sekunde, auch wenn ein Bild länger dauerte. Seine warmen Vergleiche
bleiben historische Messungen; sie belegen nicht die erste Reaktion der App.

## Erfasster Ablauf

`secondbrain-ui-profile` erfasst jetzt Ereignisverarbeitung, Tick/Layout,
Zeichnen, Present und nachfolgende Modellaktionen getrennt. Die Gesamtdauer
enthält diese fünf Abschnitte. Start und Beenden werden zusätzlich gemessen.
Der Performance-Counter bleibt über `SDL_Quit` hinweg verwendbar; die
SDL-Tickzeitbasis würde beim Beenden zurückgesetzt. Eine frühe Diagnosefassung
lieferte deshalb falsche Shutdown-Werte und ist dafür kein Nachweis.

Jede der vier Phasen erhält 68 Bilder: Ruhe, Scrollen, Kamerabewegung und
Dokumentwechsel. Die bisherige Zusammenfassung der 60 Bilder ab Index 8 bleibt
erhalten; zusätzlich erscheint das langsamste der ersten acht Bilder. Der
optionale CSV-Export enthält alle 272 Bilder, die verlangte Notiz, den
Modellpfad davor/danach, Reader-Zähler und den tatsächlichen Tickabstand.
Nur das erste Bild jeder Phase startet mit 1/60 Sekunde; danach folgt die
Animation der gemessenen verstrichenen Zeit wie in der produktiven Schleife.

Die CSV-Datei muss neu sein. Ein vorhandener Pfad wird abgewiesen und seine
Bytes bleiben erhalten. Komma und Anführungszeichen in Notizpfaden sind als
CSV-Felder maskiert. Ohne `--trace` funktioniert der bisherige Aufruf weiter.

## Wiederholen

Aus dem Repository-Ordner mit einem vorhandenen Release-Build `build/current-ui`:

```sh
cmake --build build/current-ui --target secondbrain-ui-profile --parallel 4
python3 -c "import shutil; shutil.copytree('brains/secondbrain', 'build/frame-timing-demo/workspace/secondbrain')"
build/current-ui/secondbrain-ui-profile \
  build/frame-timing-demo/workspace secondbrain assets/fonts/NotoSans-Regular.ttf \
  native --trace build/frame-timing-demo/frames.csv \
  journal/2026-10-07.md inbox/INBOX.md START.md
```

Die Kopie darf noch nicht existieren; der Kopierbefehl überschreibt sie nicht.
`software` wählt statt `native` den SDL-Softwarerenderer. Messläufe erfolgen
nacheinander, ohne gleichzeitige Builds oder weitere App-Prüfungen.

## Ausgeführte Prüfung

Drei serielle Metal-Läufe und ein Softwarelauf auf macOS 14.6.1/23G93,
Intel-Mac, Fenster 1336×840. Die erste Notiz ist die
62.212 Bytes lange Übergabe vom 7. Oktober. Eine zusätzliche Diagnose-Notiz
mit Komma und Anführungszeichen im Pfad prüft den Export. Alle 19 Dateien der
Kopie und die 18 ursprünglichen Projektgedächtnisdateien bleiben unverändert.

| Lauf | Initialisierung | Erstes Bild der langen Notiz | Größter Frame beim Wechsel | Beenden |
| --- | ---: | ---: | ---: | ---: |
| Metal 1 | 179,07 ms | 126,26 ms | 33,30 ms | 12,74 ms |
| Metal 2 | 226,07 ms | 125,65 ms | 43,50 ms | 11,23 ms |
| Metal 3 | 211,34 ms | 113,91 ms | 27,12 ms | 10,35 ms |
| Software | 183,25 ms | 166,12 ms | 108,14 ms | 21,60 ms |

Initialisierung enthält Fenster, Schriften und Modellaufbau; das Modell kann
dabei bereits das erste Projekt öffnen. Der anschließende Projektaufruf ist
in dieser Fixture daher praktisch ohne zusätzliche Arbeit. Die erste
Bilddauer enthält nicht diese Initialisierung und nicht den vorherigen
Notizaufruf. Ein neuer Prozess bedeutet keinen kalten Datenträgercache.

Der Prozessprüfer kontrolliert 272 aufeinanderfolgende Bildwerte pro Lauf,
nichtnegative Abschnittszeiten und deren Summe, sieben angenommene
Notizanforderungen, CSV-Pfade, reale Tickabstände, unveränderte Dateien und
die Abweisung bestehender Ausgaben. Ein zusätzlicher Softwarelauf prüft den
bisherigen Aufruf ohne Export; ungültige Argumente werden vor dem Fensterstart
abgewiesen. Release-Build und `git diff --check` bestehen. Die Änderung betrifft
das Diagnosewerkzeug; die 78 Produktprüfungen von 0.9.55 sind frühere Nachweise
und wurden für diesen Schritt nicht erneut ausgeführt.

Originale: `build/frame-timing-verification/final/`, insbesondere `verify.py`,
`verify.log`, `results.json` und die vier CSV-/Textprotokolle. Vorläufige
Messungen in den übergeordneten Verzeichnissen ersetzen diese Schlussprüfung
nicht.

## Grenzen und nächste Untersuchung

Der Prüfer fordert fortlaufend Bilder mit einem 60-Hz-Budget an; er bildet
keinen ruhenden Desktop mit dessen längeren Ereigniswartezeiten nach.
Protokollausgabe und künstliche Wartezeit liegen außerhalb der Frame-Gesamtdauer,
fließen aber in den nächsten realen Tickabstand ein. Messung und Ausgabe haben
eigenen Aufwand. GPU-Arbeit kann erst bei späteren Aufrufen warten; die fünf
Abschnitte sind kein unabhängiges GPU-Profil.

Ein Modellpfad nach `apply` beweist die angenommene Auswahl, nicht den Zeitpunkt
des erstmals sichtbaren fertigen Texts. Dafür fehlen noch passende Bildschirm-
und Eingabeabnahmen. Die warmen Gesamtdauern sind durch die hinzugefügte
Eingabezeit und die tatsächlichen Animationsabstände nicht unverändert mit
früheren Prüferausgaben vergleichbar.

Die Messungen zeigen weiter Arbeit beim ersten Bild sowie Layout-/Zeichenspitzen
in Bewegungsphasen. Als Nächstes sind die wiederholte Inline-/native
Textaufbereitung und der erste Quell-/Parser-/Vorbereitungspfad getrennt zu
profilieren. Schwächere Geräte, Windows/Linux/ARM64, große reale Wissensbasen
und menschliche Bedienung bleiben offen. SBUI-074 und die vollständige
[Release-Liste](RELEASE.md) sind damit nicht abgeschlossen.
