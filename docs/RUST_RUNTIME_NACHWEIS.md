# Rust-Laufzeit der Zugänglichkeitsbibliothek

Stand: 8. Oktober 2026. Rust gehört zur externen UI-Anbindung AccessKit.
Die eigene Anwendung und ihr fachlicher Kern bleiben C17.

## Zwei beobachtete Compilerstände

Die festgelegte macOS-AccessKit-Bibliothek enthält Quellpfade mit Rust-Commit
`48a229ceaefd4985c50990b14116b6d856af0985`. Dessen Original `src/version`
nennt 1.98.1. Diese vorgebaute Bibliothek verwendet die installierte App 0.9.21.

Die lokale Cargo-Probe aus build/accesskit-cache-research/cargo-build enthält
einen erfolgreichen Compiler-Versionsabruf: 1.99.0, Commit
`b940084d7eb6a299eb4bfeb8e34901bc051e7ac4`, Host x86_64-apple-darwin,
LLVM 23.1.2. Diese Angabe stammt aus dem Cache des ausgeführten Builds.

Windows und Linux bauen AccessKit aus der festgelegten Quelle. Deshalb werden
deren tatsächliche Compilerstände künftig nach dem Build aus Cargos
`.rustc_info.json` erhoben und als ui-rust-compiler.json im CI-Nachweis gespeichert.
Ein zusätzlich aufgerufenes System-rustc könnte bei Overrides einen anderen
Compiler nennen; der Nachweis verwendet den erfolgreichen Cargo-Versionsabruf.

[Erfassung](../tools/record_ui_rust_compiler.py) prüft genau eine erfolgreiche
Identität mit vollständigem Commit, Release und Host und hält den Hash der
Original-Cachedatei fest. Fehlende oder unvollständige Daten werden abgewiesen.
Die Aufnahme identifiziert den Compiler; sie bescheinigt noch keine vollständige
Lizenzzuordnung oder kryptografisch attestierte Buildherkunft.

## Zusätzlicher Quellumfang

Die originale [Library-Lockdatei zum beobachteten macOS-Commit](https://github.com/rust-lang/rust/blob/48a229ceaefd4985c50990b14116b6d856af0985/library/Cargo.lock)
hat SHA-256 `d1c5dbdf53bfebd7de60f26a171819db3b28ebfd75f3fa99c3286893a5a7b7a6`.
Sie enthält 30 Registry-Pakete und zusätzliche In-tree-Pakete, darunter
compiler_builtins 0.1.160. Dazu gehören Versionsstände von addr2line, gimli,
object, miniz_oxide und rustc-demangle, die die bisherige AccessKit-Inventur
nicht vollständig abdeckt.

Die Lockdatei schließt Tests, Buildfunktionen und andere Zielsysteme ein.
Die 30 Pakete werden deshalb nicht pauschal als in der macOS-App enthalten
ausgewiesen. Zielabhängigkeiten und optionale Backtrace-Funktionen stehen im
[std-Manifest](https://github.com/rust-lang/rust/blob/48a229ceaefd4985c50990b14116b6d856af0985/library/std/Cargo.toml);
die Auswahl der Unwinder im
[unwind-Manifest](https://github.com/rust-lang/rust/blob/48a229ceaefd4985c50990b14116b6d856af0985/library/unwind/Cargo.toml).

Rusts [Original-COPYRIGHT](https://github.com/rust-lang/rust/blob/48a229ceaefd4985c50990b14116b6d856af0985/COPYRIGHT)
verweist auf In-tree-Metadaten und Cargo-Abhängigkeiten sowie die erzeugten
Copyrightberichte. Der lokale Homebrew-Bericht COPYRIGHT-library.html hat
einen leeren Abschnitt für externe Abhängigkeiten. Sein Inhalt allein schließt
die zusätzliche Prüfung deshalb nicht ab.

## Geprüft und offen

Eine echte, isolierte Cargo-Kompilierung ohne Registry-Abhängigkeiten erzeugt
den Cache. Die erfasste Ausgabe stimmt mit dem Compilerabruf in derselben
Umgebung überein. Eine fehlende Identität wird abgewiesen. Beide Prüfungen
stehen in [test_ui_rust_compiler.py](../tests/test_ui_rust_compiler.py).

## Mitgelieferte Originale ab 0.9.22

[Rust-runtime.txt](../third_party/licenses/Rust-runtime.txt) und
[Manifest](../third_party/rust-runtime-manifest.json) ergänzen die Lizenzansicht.
Die Original-Lockdateien und offiziellen rust-src-Archive für 1.98.1/1.99.0
stimmen überein. Cargo-Metadaten wurden offline und locked für die vier
unterstützten Zielarchitekturen mit allen Cargo-Features erhoben. Ausgehend von
std werden normale und Buildabhängigkeiten verfolgt; reine Testabhängigkeiten
werden ausgelassen. Das ist ein konservativer Umfang für die unterstützten
Ziele, keine Behauptung über jedes tatsächlich gelinkte optionale Feature.

Je Quellstand ergeben sich 13 Registry-Komponenten für macOS ARM64/Intel und
Linux x64 sowie 6 für Windows MSVC x64. Über beide Stände sind es 20 verschiedene
Name-/Versionspaare. Der vollständige Library-Lockumfang von je 30 Registry-
Paketen bleibt davon unterschieden. Fremde Zielsysteme wie SGX/VEX werden nicht
als App-Abhängigkeiten ausgegeben.

103 unterschiedliche Original-Payloads enthalten Rootlizenzen, Registry-
Originale und zusätzliche Quellhinweise, darunter 305 Archiv-Quellrecords.
Die Archive sind durch ihre offiziellen SHA-256-Werte und die darin liegende
Lockdatei geprüft. Crate-Archive werden vor dem Lesen gegen die Original-Lock-
Prüfsummen geprüft. In-tree-Originale haben zusätzliche feste Dateihashes.
Der Collector extrahiert keine ausführbaren Dateien und führt Quellmaterial
nicht aus. Alle Texte bleiben in der App Literaltext.

compiler-builtins nennt zusammengesetzte AND-Bedingungen einschließlich der
LLVM-Ausnahme. Der vollständige [Originaltext](https://github.com/rust-lang/rust/blob/48a229ceaefd4985c50990b14116b6d856af0985/library/compiler-builtins/LICENSE.txt)
und die unveränderte Lizenzdeklaration bleiben enthalten; sie werden nicht auf
MIT verkürzt. Der LLVM-libunwind-Originaltext und Copyright-Kommentare stammen
aus den geprüften Rust-Quellarchiven. Quellenhinweise schließen konservativ
auch inaktive In-tree-Quell-/Testabschnitte ein.

[Collector](../tools/collect_rust_runtime_notices.py) ist ein Entwicklerwerkzeug
mit Python 3.11 oder neuer. Es braucht pro Compilerstand vier Metadatendateien,
die originale Library-Lockdatei und das geprüfte rust-src-Archiv. Unbekannte
Stände oder geänderte Originale werden abgewiesen. Eine neue Toolchain braucht
einen neuen Quellenabgleich; dieser Snapshot gilt nicht pauschal für beliebige
Rust-Installationen.

[Integritätsprüfung](../tests/test_rust_runtime_inventory.py) prüft außerdem
die einzelnen Original-Payloads, selbst bei neu berechnetem Gesamthash, die
Zielabdeckung und die AND-Bedingung. Corpus-/App-/Paketnachweise stehen in
[STATUS](STATUS.md).

CI [37695359274](https://github.com/Lulus792/SecondBrain/actions/runs/37695359274)
beobachtet auf Windows und Linux tatsächlich Rust 1.98.1/48a229c.
Der Abgleich sämtlicher tatsächlich gelinkter In-tree-/SDK-/Systemruntime-
Anteile sowie abweichender Toolchain-Konfigurationen bleibt offen.
Die vollständige transitive Release-Abnahme wird durch diesen Schritt nicht
als abgeschlossen markiert.
