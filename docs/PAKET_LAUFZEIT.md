# Laufzeitabhängigkeiten der Pakete

Stand: 8. Oktober 2026. Die Paketprüfung untersucht die tatsächlich entpackten
Programme, bevor sie ihre normalen Abläufe startet. Python, CMake und native
Binärwerkzeuge gehören zum Entwicklerprüfweg; die App benötigt sie nicht.

[Scanner](../tools/package_runtime.py) und [CMake-Auflösung](../tools/package_runtime.cmake)
verwenden die verlinkten Imports von App und CLI. Jedes Programm wird getrennt
aufgelöst: Gleich benannte Bibliotheken in zwei legitimen Programmordnern sind
dadurch kein pauschaler Konflikt. Der Bericht enthält Binärhashes, aufgelöste
Dateien, Konflikte, nicht aufgelöste Namen und Kategorien der Abhängigkeiten.

## Kategorien und Grenzen

- Dateien innerhalb des entpackten Pakets sind mitgeliefert. Symlinks werden
  dabei aufgelöst; ein Link in einen fremden Entwicklerordner zählt nicht als
  mitgelieferte Bibliothek.
- macOS-Systembibliotheken und Frameworks unter /usr/lib und /System/Library
  sind OS-Verträge. Viele liegen im dyld-Cache. Der Scanner zeichnet direkte
  Systemimports zusätzlich mit otool auf und prüft alle übrigen Auflösungen.
- Windows-API-Sets sind virtuelle OS-Verträge. Andere DLLs werden aufgelöst;
  bekannte Visual-C++-Laufzeitnamen gelten auch aus System32 als separate
  Redistributable-Abhängigkeit. Ihre Verfügbarkeit auf einem Entwickler-Runner
  belegt keine saubere Nutzerinstallation.
  Der Resolver stoppt an der Windows-Systemgrenze. Die internen Imports von
  OS-Implementierungs-DLLs, etwa optionale Azure-/Hvsi-/Sensor-Komponenten,
  gehören zur OS-Wartung. Bekannt separat redistributable CRT-Dateien bleiben
  ausdrücklich von dieser Ausschlussregel ausgenommen. Beide Pfadtrenner sind
  in der tatsächlichen CMake-Regel geprüft.
- Linux-Bibliotheken unter den regulären lib-/usr-lib-Verzeichnissen gehören
  zum System-/Desktopbedarf. Die benötigten Dateien stehen im Bericht.
  Verfügbarkeit auf der zugesagten Distribution muss gesondert geprüft werden.

Nicht aufgelöste Imports, Konflikte innerhalb einer Programmauflösung und
weitere externe Laufzeitdateien lassen die Paketprüfung fehlschlagen. Der
Bericht wird trotzdem für die Diagnose geschrieben und in CI gespeichert.
Die Verzeichniszuordnung ist keine kryptografische Herkunftsprüfung aller
Systemdateien. Optionale über dlopen/LoadLibrary geladene Renderer-/Gerätetreiber,
OS-Mindestversionen und frische Nutzerrechner bleiben eigene Abnahmen.

Grundlage: [CMake Runtime Dependencies](https://cmake.org/cmake/help/latest/command/file.html#get-runtime-dependencies).
Für Visual-C++-Dateien müssen zusätzlich die tatsächlichen
[Microsoft-Redistributionsbedingungen](https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files?view=msvc-170)
und der gewählte Installationsweg erfüllt sein. Der Scanner kopiert keine
System-/Redistributable-Dateien automatisch in ein Paket.

## Tatsächliche Nachweise

Ein eigenes C-Fixture baut eine echte Shared Library und zwei Programme.
Die Prüfung akzeptiert mitgelieferte Bibliotheken; nach deren Entfernen wird
das Paket abgewiesen. Unter macOS liegen zwei gleichnamige Bibliothekskopien
neben den jeweiligen Programmen, sodass die getrennte Auflösung geprüft ist.
[Fixture](../tests/test_package_runtime.py). Weitere native Plattformnachweise
werden erst aus ausgeführten CI-Läufen übernommen.

Installiertes Intel/macOS-Paket 0.9.22/4e6f15c86d71: keine nicht aufgelösten,
externen oder konfliktbehafteten verlinkten Nicht-Systembibliotheken gefunden.
Die Systemimporte sind im lokalen Bericht aufgeführt. LC_BUILD_VERSION nennt
für App und CLI macOS 14.0 und SDK 15.2. Das ist Binärmetadaten-Nachweis; die
eigene bisherige lokale Ausführung ist auf macOS 14.6.1 erfolgt. Eine tatsächlich
geprüfte Mindestversion für 1.0 wird daraus nicht vorweggenommen.

Berichte: build/package-runtime-local.json und
build/dependency-package-check/runtime-dependencies.json. Nachweise des
vollständigen Paketablaufs stehen in [STATUS](STATUS.md).

## MSVC-UI-DLL ab 0.9.23

Die [Windows-Diagnose zu e4feaf6](https://github.com/Lulus792/SecondBrain/actions/runs/37701935703)
beobachtet VCRUNTIME140 als externe Laufzeit der tatsächlich gebauten DLL.
AccessKit erhält deshalb für MSVC ein explizites Ziel und das Ziel-Flag
`-C target-feature=+crt-static`. Die UI-DLL bleibt eine separate DLL mit eigenem
Allocator; die C-ABI-Grenze und ihre Freigabefunktionen bleiben erhalten.
Die C/C++-App verwendet bereits /MT. Die MinGW-Konfiguration bleibt eine
gesonderte Toolchain-Variante.

Grundlagen: [Rust CRT-Verknüpfung](https://doc.rust-lang.org/reference/linkage.html#static-and-dynamic-c-runtimes)
und [zielbezogene Cargo-Flags](https://doc.rust-lang.org/cargo/reference/config.html#targettriplerustflags).
[CI 37704298271](https://github.com/Lulus792/SecondBrain/actions/runs/37704298271)
zu 7f9acb8 bestätigt die neue Konfiguration mit erfolgreicher Windows-Release-
Importprüfung und anschließender Paket-/UI-Abnahme. Alle 20 Jobs und alle vier
entpackten Release-Pakete bestehen. Im geprüften Windows-Paket bleibt keine
externe Visual-C++-Runtime erforderlich. Der lokale 0.9.23-Mac-Importbericht
steht in build/crt-package-check/runtime-dependencies.json. Optionale dynamische
Imports und frische Nutzerrechner sind davon nicht abgedeckt.

## ELF-Anforderungen vom 11. Oktober

Der Linux-Prüfweg erfasst zusätzlich Architektur, direkte `NEEDED`-Bibliotheken
und die **Version References** aus `objdump -p`. Alle Versionstags bleiben mit
Bibliotheksnamen und Flags erhalten. Die höchsten numerischen GLIBC-, GLIBCXX-,
CXXABI- und GCC-Tags werden getrennt ausgewertet. Exportierte Versionen werden
nicht als Anforderungen gezählt; ein Vergleich als Zeichenkette würde etwa2.9
fälschlich über2.10 einordnen. Unvollständige oder unbekannte Tabellen brechen
die Prüfung ab. Eine reine Suche nach undefinierten Symbolen würde unter anderem
Copy-Relocations übersehen. Grundlage für das Binärwerkzeug:
[LLVM objdump](https://llvm.org/docs/CommandGuide/llvm-objdump.html).

Tatsächlich untersucht ist das unveränderte Linux-x86_64-Paket0.9.64 zu
`bef4816d3e403ad7d6719366349030e6273707ef`, CI38171616070,
Artefakt11714189375. Der heruntergeladene ZIP-Hash entspricht dem GitHub-Digest:
`950347bfc983ae253fddc52ba16e7fb3784eb357ed9e893f43f9389b39efe9f2`.

| Programm | Höchster direkter GLIBC-Tag | Weitere Anforderungen |
| --- | --- | --- |
| App | GLIBC_2.39 | GCC_4.2.0, LIBDBUS_1_3; vollständige Tagliste im Bericht |
| CLI | GLIBC_2.34 | libc.so.6; vollständige Tagliste im Bericht |

Das Paket wurde auf dem Intel-Mac nur gelesen, nicht als Linux-Programm
ausgeführt. Seine bereits bestandene native Linux-Paketprüfung gehört zum
ursprünglichen CI-Lauf. Der neue Scanner benötigt noch seine eigene native
Linux-CI-Prüfung mit GNU objdump. Lokal bestehen sechs Python-Prüfungen;
eine Windows-spezifische Prüfung ist entsprechend übersprungen. Die komplette
macOS-Abhängigkeitsprüfung des installierten0.9.64-Pakets besteht weiterhin,
mit `minos14.0` und SDK15.2 für beide Programme.

Die Tags beschreiben die direkten Anforderungen der zwei Programme. Sie sind
kein vollständiger Kernel-/Distributionsvertrag. Transitive Systembibliotheken,
dlopen-Komponenten, frische Desktopinstallationen und tatsächliche
Mindestversionsbedienung bleiben offen. Die App-Version bleibt0.9.64;
geändert sind Entwicklerprüfwerkzeuge, keine Anwendungsfunktionen.
Originale: `build/runtime-abi-verification/linux-abi.json`,
`linux-headers.txt`, `linux-symbols.txt` und `macos-runtime.json`.
