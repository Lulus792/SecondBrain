# Rust-Laufzeit der Zugänglichkeitsbibliothek

Stand: 7. Oktober 2026. Rust gehört zur externen UI-Anbindung AccessKit.
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

Als Nächstes werden Originaltexte der passenden Rust-Laufzeitabhängigkeiten,
compiler-builtins und gegebenenfalls LLVM-/System-Unwinder zugeordnet und in
App/Pakete übernommen. Dazu kommen die tatsächlich beobachteten Windows-/
Linux-Compilerstände. Die vollständige transitive Release-Abnahme bleibt offen.
