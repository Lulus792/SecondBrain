# Tatsächlich eingebundene Objektdateien

Stand: 12. Oktober 2026. Linkerkarten ergänzen die Import- und Laufzeitprüfung
für den noch offenen Abgleich von Compiler-/SDK-Anteilen. Eine Karte benennt
Herkunft und Positionen von Symbolen; sie ersetzt weder Original-Lizenztexte
noch die Prüfung der tatsächlichen Paketdateien.

Grundlagen, am 12. Oktober gelesen: Apples
[Beispiel zur Symbolherkunft](https://developer.apple.com/forums/thread/733475),
die installierte AppleClang-16-Linker-Manpage, Microsofts
[/MAP](https://learn.microsoft.com/en-us/cpp/build/reference/map-generate-mapfile?view=msvc-170)
und die [GNU-Linkeroptionen](https://sourceware.org/binutils/docs/ld/Options.html).

## Vorhandene Intel/macOS-App

Die installierte 0.9.74/eabf5b3ff114 wurde mit ihrem vorhandenen Linkbefehl in
einem getrennten Diagnoseordner erneut gelinkt, ergänzt um `-map`. Die Ausgabe
ist **bytegleich** mit der installierten App:
`d0b039034dce04ca8a5cf712bdc7858810040008aeb1715acb2dfc710fc65ff7`.
Damit gehört die untersuchte Karte zur tatsächlichen App, nicht nur zu einer
ähnlichen Quellenfassung. Der separate CLI-Versuch unterscheidet sich im UUID-
Feld; nur nach Auslassen dieser 16 Bytes sind die Dateien gleich. Er wird
ausdrücklich nicht als bytegleicher CLI-Nachweis behandelt.

Die App-Karte nennt 167 SDL3-, 31 SDL_ttf-/Textbibliotheksobjekte, eigene
Bibliotheksobjekte, einen zusammengefassten AccessKit-/Rust-Beitrag sowie 23
SDK-Importstubs. Die Stubs sind keine mitgelieferten Kopien der Systemframeworks.
Ein separates statisches `libclang_rt.osx.a`-Objekt erscheint im Kartenindex
nicht; `libcompiler_rt` wird über einen SDK-Systemimport benannt. Daraus folgt
keine vollständige Aussage über die Interna des zusammengefassten Rust-Objekts.
Diese werden weiterhin mit dem [Rust-Nachweis](RUST_RUNTIME_NACHWEIS.md) abgeglichen.

Originale: `build/label74-toolchain-map/identity.json`, `objects.json`,
`cli-comparison.json` und die beiden unveränderten Rohkarten. Der gesamte
Symbolbereich ist nicht durchgehend gültiges UTF-8; die Object-files-Tabelle
wurde getrennt und strikt gelesen, die Rohdaten bleiben erhalten.

## Reproduzierbare Build-Nachweise

`-DSB_RECORD_LINK_MAPS=ON` erzeugt Karten für App und CLI unter
`buildordner/link-maps/Konfiguration/`. Die Option ist standardmäßig aus;
der geprüfte UI-CI-Konfigurationsweg aktiviert sie. MSVC verwendet `/MAP`,
AppleClang `-map`, GNU/Clang `-Map`. Unbekannte Toolchains werden bei aktivierter
Option abgewiesen. Alte Karten werden vor dem Linken entfernt; anschließend
müssen Programm und nichtleere Karte existieren. Eine `.map.identity` bindet
beide über SHA256. Die CI bewahrt Karten und Identitäten mit den Nachweisen auf.

Auf Intel/macOS bestehen Konfiguration, tatsächliches Linken, Hashvergleich
beider Karten/Programme, Abweisung einer leeren Karte und beide Versionsprüfungen
sowie normaler Metal-Start (drei CTest-Gruppen, 1,22 s). Ein tatsächliches
Diagnosepaket enthält keine Karten und seine zwei Programme stimmen mit den
Build-Identitäten überein. Originale: `build/label74-toolchain-map/`.

## Native Karten und tatsächliche Pakete

[CI38199346371](https://github.com/Lulus792/SecondBrain/actions/runs/38199346371)
zu `325f86d31e27cc79624a26c2b6d6c05c9f0af382` besteht unter Windows/Linux in
Debug und Release mit jeweils 90 Tests und beiden Release-Paketen. Mac-Jobs
warten auf Runner. Tatsächliche C-/C++-Compiler: MSVC 19.51.36260.0 beziehungsweise
GNU 13.3.0. Diese Abnahme gehört zum optionalen Karten-Buildweg und ist getrennt
vom zuvor installierten Produktbuild eabf5b3.

Alle vier heruntergeladenen Paket-/Nachweis-ZIPs stimmen mit den GitHub-Digests
überein. Beide Karten pro Plattform stimmen mit ihren `.identity`-Hashes überein;
App und CLI im tatsächlichen Paket stimmen mit den jeweiligen Programm-Hashes
überein. Karten/Identitäten sind in den Entwicklungsnachweisen und fehlen in den
Anwendungspaketen. Die fremden Programme wurden auf dem Mac gelesen, nicht ausgeführt.

| Plattform | Paket-Artefakt | Nachweis-Artefakt |
| --- | --- | --- |
| Linux | 11724029521 | 11724388310 |
| Windows | 11724953387 | 11724569670 |

Originale und SHA256: `build/link-map-native/`, insbesondere `native-ci.json`,
`linux-map-crosscheck.json`, `windows-map-crosscheck.json` und die unveränderten
ZIPs/Karten. Die Windows-Karte benennt in ihren Symbolursprüngen folgende
eindeutige Objektmitglieder; das ist keine vollständige private Symbolinventur:

| Beitrag | App | CLI |
| --- | --- | --- |
| LIBCMT | 42 | 37 |
| libucrt | 243 | 154 |
| libvcruntime | 30 | 25 |
| uuid | 2 | 0 |

Unter Linux sind `Scrt1.o`, `crti.o`, `crtn.o`, `crtbeginS.o` und `crtendS.o`
benannt, einschließlich nichtleerer Beiträge zur ausgegebenen Datei. `libgcc.a`
steht in LOAD-Zeilen, aber es sind keine eindeutig extrahierten Mitglieder
`libgcc.a(...)` benannt. LOAD allein wird deshalb nicht als Nachweis einer
eingebetteten statischen libgcc behandelt.

## Noch erforderlicher Quellenabgleich

Die erkannten Beiträge bestimmen die nächsten Originalquellen: unter Windows
das [Lizenzverzeichnis](https://visualstudio.microsoft.com/license-terms/),
die [Verteilungsliste für Visual Studio 2026](https://learn.microsoft.com/en-us/visualstudio/releases/2026/redistribution)
und die tatsächlich verwendeten SDK-Dateien; unter Linux die konkreten
Distributionspakete der Startobjekte. Microsofts
[Compilerzuordnung](https://learn.microsoft.com/en-us/cpp/overview/compiler-versions?view=msvc-170)
ordnet 1951/14.51 den neueren Toolsets zu. Eine pauschale Zuordnung zu Visual
Studio 2022 wäre für diesen beobachteten Compiler ungeeignet.

Die offiziellen 2026-Enterprise-/Runtime-Dokumente, die Verteilungsliste und
die Compilerzuordnung wurden am 12. Oktober im betreffenden Umfang gelesen.
GCCs Original `libgcc/crtstuff.c` zu 13.3.0 nennt GPLv3 mit Runtime Library
Exception 3.1; `COPYING.RUNTIME` wurde ebenfalls gelesen. Das ist ein
Upstream-Quellenkandidat, noch kein Hash-/Lizenzabgleich mit den konkreten
Ubuntu-Paketdateien. Rohquellen/Hashes: `build/link-map-runtime-review/`.
Systemruntimes, konkrete SDK-/Toolchainbedingungen und innere Rust-Anteile
bleiben gezielt abzugleichen; dies ist keine vollständige Lizenzfreigabe.
Die installierte App bleibt die bereits geprüfte saubere 0.9.74/eabf5b3ff114.

## Konkrete installierte Linux-Startobjekte

`tools/record_linux_startup.py` ergänzt den CI-Build um die nach dem Linken
beobachteten Dateihashes, Debian-Binär-/Quellpaketnamen, Versionen, Architektur
und originale installierte Copyrightdateien. Es liest ausschließlich die durch
Programm-/Kartenhashes gebundenen GNU-Ausgaben von App und CLI. Verworfene
Abschnitte, LOAD-Zeilen, leere Beiträge und Debug-/Kommentarabschnitte zählen
nicht als eingebundene Startobjekte. Pfade werden vor der Paketzuordnung aufgelöst;
Objekt- und Copyrightbytes werden während des gesamten Durchlaufs nachgeprüft.
Eine vorhandene Ausgabedatei wird geschützt. Die Lizenztexte bleiben Quelldaten,
keine ausführbaren Anweisungen; dies verändert keine Lizenz der Anwendung.

Grundlage, am 12. Oktober gelesen:
[dpkg-query](https://manpages.debian.org/bookworm/dpkg/dpkg-query.1.en.html),
insbesondere genaue Dateisuche, Multi-Arch-Paketnamen, `source:Package` und
`source:Version`. Die Abfragen benutzen C.UTF-8 und übergeben Argumente direkt,
ohne Shell-Auswertung. Der Bericht enthält installierte Quellenhinweise, aber
noch keinen Abgleich mit heruntergeladenen ursprünglichen Source-Paketen.

Lokal bestehen vier Prüffälle, die Einträge ohne wirkliche Beiträge sowie
geänderte Programm-/Kartenidentitäten abweisen und Paket-/Quellidentitäten
erhalten. Der neue CTest-Vertrag besteht; die Python-Gesamtprüfung besteht mit
68 Tests, davon eine plattformspezifische Prüfung übersprungen. Zusätzlich
erkennt derselbe Parser in den tatsächlich heruntergeladenen Linux-Karten je
fünf Startobjekte. Das ist ein Parsernachweis auf dem Mac, keine dort ausgeführte
Debian-Paketabfrage. Die neue tatsächliche Linux-CI-Abnahme folgt.
Originale: `build/startup-inventory-check/`. Die installierte App bleibt unverändert.
