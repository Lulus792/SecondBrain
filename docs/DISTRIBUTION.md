# Anwendungspakete

SecondBrain ist eine lokale Desktop-Anwendung in Entwicklung. Zum Bauen werden ein
C17-Compiler, ein C++-Compiler für die UI-Textbibliothek und CMake ab 3.24 benötigt. Zum Starten des fertigen Pakets sind
Python, CMake und ein Compiler nicht erforderlich. Automatisierte Prozessprüfungen
beim Bauen benötigen Python 3; `-DBUILD_TESTING=OFF` baut ohne diese Testwerkzeuge.

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
Maus-/Tastatur-Bedienablauf und zusätzlich einen reinen Tastaturdurchlauf. Schriften werden anhand des Anwendungsordners gesucht; eine
Referenz auf den ursprünglichen Quellordner ist nicht erforderlich.
Die Entwickler-Paketprüfung benötigt Python 3 und die nativen Werkzeuge
otool/dumpbin/objdump. Sie prüft außerdem die
[verlinkten Laufzeitabhängigkeiten](PAKET_LAUFZEIT.md) des entpackten Pakets.

## Inhalt

Windows erhält ein ZIP mit `secondbrain.exe` und `assets`. macOS erhält ein
tar.gz mit `secondbrain.app`, dessen Ressourcen im Bundle liegen. Linux erhält
ein tar.gz mit `secondbrain` und `assets`. Alle Pakete enthalten außerdem das
native Entwicklungswerkzeug, `QUICKSTART.txt` und die UI-/Schriftlizenzen.
Der normale Zugang zu den Projektgedächtnissen ist die eigene Oberfläche.

SDL wird statisch eingebunden. MSVC baut die C-Laufzeit statisch ein. Der eigene
C-Kern verwendet ausschließlich C-Standardbibliothek und Betriebssystem-APIs.
Bestehende Markdown-Instanzen werden beim Paketwechsel nicht überschrieben.
Ab 0.9.23 wird auch die aus Rust gebaute MSVC-UI-DLL mit statischer CRT
vorbereitet. Den tatsächlich bestätigten DLL-/Paketstand nennt der
[Laufzeitnachweis](PAKET_LAUFZEIT.md); Konfiguration allein belegt die Verknüpfung nicht.

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
AccessKit ergänzt die native Zugänglichkeit. Vollständige Dokumentsemantik
und menschliche Screenreader-Abnahme bleiben offen.

KI-Kontext wird aus den gespeicherten Kerninformationen kopiert. Weitere
Notizen sind über die Projektdateien erreichbar. Ein KI-Chat muss das Lesen und
die Pflege ausdrücklich übernehmen; automatische Synchronisation und eine
integrierte Verbindung zu einem KI-Anbieter sind spätere Erweiterungen.

Die Sternkarte ist auf 4096 Dokumente und 65536 interne Verweise je Projekt begrenzt.
Bei Überschreitung bleibt die Dokumentliste verfügbar. Die Graphauswertung
unterstützt einfache Markdown-Inline-Links; Referenzdefinitionen und Wiki-Links
werden noch nicht ausgewertet. Die Glasdarstellung ist ein eigener C-Effekt und
verwendet keine native macOS-26-Materialkomponente. Reduzierte Transparenz lässt
sich in der App aktivieren. Kamera und Scrollen verwenden kurze Übergänge;
„Bewegung reduzieren“ schaltet sie ab. Es gibt keine automatische Rotation.


## Linux-UI-Abhängigkeit bauen

Ab 0.5.2 wird die AccessKit-Linux-Bibliothek aus festgelegter Quelle gebaut, um
ihre AT-SPI-Cache-Signale zu korrigieren. Entwickler benötigen dafür zusätzlich
Cargo/Rust ab 1.87. Windows baut ab 0.9.13 ebenfalls die festgelegte UI-DLL
aus Quelle, einschließlich der [UIA-Textsuche](UIA_TEXTSUCHE.md). Der Anwendungscode und fachliche Kern bleiben C. Ein Kernbuild
mit SB_BUILD_UI=OFF benötigt keine Rust-Toolchain; fertige Pakete ebenfalls nicht.
Quellen, Hashes und Umfang stehen in [third_party](../third_party/README.md).

Ab 0.6.0 gehören libdbus-1 und GIO/GLib zur Linux-UI-Systemanbindung. Zum Bauen
werden ihre Entwicklungsdateien benötigt (libdbus-1-dev, libglib2.0-dev); zur
Laufzeit die üblichen Desktop-Systembibliotheken. Ein reiner C-Kernbuild bleibt
unabhängig davon. Die Bibliothekskopien werden nicht im Anwendungspaket gebündelt.


## Dauerhafte Vorabversionen vorbereiten

Der neue Workflow prerelease.yml akzeptiert ausschließlich Tags v0.MINOR.PATCH,
die zur CMake-Version passen. Er verwendet dieselben vollständigen Plattform-
prüfungen wie die normale CI, einschließlich zusätzlich geplantem Intel-Mac-Runner.
Erst danach werden vier Archive mit SHA256SUMS in einen GitHub-Release-Entwurf
hochgeladen, wieder heruntergeladen und gegen die lokalen geprüften Bytes verglichen.
Nur nach diesem Vergleich wird die Vorabversion veröffentlicht. Eine bestehende
Release-Version wird nicht ersetzt; ein Fehler nach dem Anlegen kann einen
unveröffentlichten Entwurf hinterlassen. Tags werden nicht erzwungen verschoben.

Die tatsächliche Erstveröffentlichung und der Intel-CI-Nachweis stehen im
Abschnitt zur veröffentlichten Vorabversion. Automatische
1.0-Veröffentlichung ist gesperrt. Signierung und Notarisierung bleiben mangels
Herausgeberzertifikaten offen. Der Release-Auftrag und die laufenden GitHub-Pushes
sind vom Nutzer autorisiert; 1.0 benötigt weiterhin seine ausdrückliche Freigabe.

## Paket aktualisieren und prüfen

App schließen und die neue Version separat entpacken. Der Arbeitsordner mit
Projektgedächtnissen bleibt erhalten. Falls nötig denselben Ordner erneut wählen.
Keine Projektmigration erfolgt automatisch. Vor größeren Änderungen lässt sich
in der App eine Sicherung erstellen. Für Prüfsummen SHA256SUMS und das passende
Archiv herunterladen; die berechnete SHA-256-Zeile mit der Zeile dieses Archivs
vergleichen. macOS: shasum -a 256 DATEI; Linux: sha256sum DATEI; PowerShell:
Get-FileHash DATEI -Algorithm SHA256.

Grundlagen am 6. Oktober 2026 gelesen:
[Workflow-Wiederverwendung](https://docs.github.com/en/actions/how-tos/reuse-automations/reuse-workflows),
[GitHub CLI Releases](https://cli.github.com/manual/gh_release_create),
[Draft-Veröffentlichung](https://cli.github.com/manual/gh_release_edit) und
[Runner-Architekturen](https://docs.github.com/en/actions/reference/runners/github-hosted-runners).

## Veröffentlicht am 7. Oktober 2026

Die [Vorabversion v0.7.3](https://github.com/Lulus792/SecondBrain/releases/tag/v0.7.3)
enthält dauerhaft vier Archive (Windows x64, Linux x64, macOS ARM64 und Intel)
und SHA256SUMS. Der [Veröffentlichungslauf](https://github.com/Lulus792/SecondBrain/actions/runs/37537383565)
besteht mit 22 Jobs einschließlich entpackter Paketprüfungen und erneutem
Herunterladen der Uploads. Tag und Quellstand d109b8a bleiben unverändert.
Die Pakete sind Entwicklungsstände ohne Herausgeberzertifikate oder Notarisierung.
