# Anwendungspakete

SecondBrain 0.1.0 ist eine lokale Desktop-Anwendung. Zum Bauen werden ein
C17-Compiler und CMake ab 3.20 benötigt. Zum Starten des fertigen Pakets sind
Python, CMake und ein Compiler nicht erforderlich.

## Paket erzeugen und prüfen

```
cmake -S . -B build/app -DCMAKE_BUILD_TYPE=Release
cmake --build build/app --config Release --parallel 4
ctest --test-dir build/app -C Release --output-on-failure
cpack --config build/app/CPackConfig.cmake -C Release -B build/app/packages
cmake -DSB_ARCHIVE_ROOT=build/app/packages -DSB_TEST_ROOT=build/app/package-check -P tests/test_package.cmake
```

Unter Linux benötigen die UI-Prüfungen eine Desktop-Sitzung oder X11 unter Xvfb.
Die CI installiert die zugehörigen Entwicklungsdateien und führt die beiden
Prüfbefehle mit `xvfb-run -a` aus. Linux zur Laufzeit benötigt weiterhin die
Systembibliotheken und eine Sitzung seines Fenstersystems.

Der Pakettest entpackt das tatsächliche ZIP beziehungsweise tar.gz in einen
neuen Ordner mit Leerzeichen und Umlauten. Er prüft Schriften, Lizenzen und
Anleitung und startet anschließend die entpackte App für den vollständigen
Bedienablauf. Schriften werden anhand des Anwendungsordners gesucht; eine
Referenz auf den ursprünglichen Quellordner ist nicht erforderlich.

## Inhalt

Windows erhält ein ZIP mit `secondbrain.exe` und `assets`. macOS erhält ein
tar.gz mit `secondbrain.app`, dessen Ressourcen im Bundle liegen. Linux erhält
ein tar.gz mit `secondbrain` und `assets`. Alle Pakete enthalten außerdem das
native Entwicklungswerkzeug, `QUICKSTART.txt` und die UI-/Schriftlizenzen.
Der normale Zugang zu den Projektgedächtnissen ist die eigene Oberfläche.

SDL wird statisch eingebunden. MSVC baut die C-Laufzeit statisch ein. Der eigene
C-Kern verwendet ausschließlich C-Standardbibliothek und Betriebssystem-APIs.
Bestehende Markdown-Instanzen werden beim Paketwechsel nicht überschrieben.

## Bezug und Grenzen

GitHub Actions prüft Desktop Debug und Release auf Windows, macOS und Ubuntu.
Nach bestandener Release- und Paketprüfung wird das jeweilige Paket als
Actions-Artefakt für 30 Tage veröffentlicht. Die Runner liefern Windows x64,
Linux x64 und macOS ARM64; lokal wurde zusätzlich Intel macOS geprüft.
Der [Plattformnachweis](PLATTFORMEN.md) nennt die tatsächlich ausgeführten Läufe.

Die Pakete sind nicht mit einem externen Herausgeberzertifikat signiert und nicht
durch Apple notarisiert. Es gibt noch keinen automatischen Installer oder
Updater. Die Tests belegen die verwendeten Runner und Abläufe, keine Freigabe für
jede ältere Betriebssystemversion.

Die App bietet grundlegende Markdown-Darstellung und UTF-8-Bearbeitung bis
16 MiB pro Datei. Rückgängig/Wiederholen ist auf 256 Operationen und 32.000
gespeicherte Unicode-Zeichen begrenzt. Die Suchnormalisierung deckt ASCII und
häufige lateinische Zeichen ab. Die mitgelieferten Schriften decken Deutsch,
Latein, Griechisch, Kyrillisch und grundlegende mathematische Symbole ab;
für weitere Schriftsysteme besteht noch kein vollständiger Schrift-Fallback.
Nuklear hat in dieser Version keine native Screenreader-Anbindung.

KI-Kontext wird aus den gespeicherten Kerninformationen kopiert. Weitere
Notizen sind über die Projektdateien erreichbar. Ein KI-Chat muss das Lesen und
die Pflege ausdrücklich übernehmen; automatische Synchronisation und eine
integrierte Verbindung zu einem KI-Anbieter sind spätere Erweiterungen.
