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

### Abgeschlossene native Linux-Inventur

[CI38201394939](https://github.com/Lulus792/SecondBrain/actions/runs/38201394939)
zu40d9271 besteht unter Windows/Linux in Debug/Release mit jeweils91 Tests und
beiden Release-Paketen. Die neue Paketabfrage wurde unter Linux tatsächlich in
Debug und Release ausgeführt: fünf Objektdateien und zwei Copyrighttexte.

Der Linux-Release-Nachweis11725208690 und das Paket11726146509 wurden gegen die
GitHub-SHA256-Digests geprüft. Der Bericht ist mit den zwei tatsächlichen
Paketprogrammen hashgleich verknüpft; die enthaltenen Original-Copyrighttexte
stimmen mit ihren aufgezeichneten Dateihashes überein. Originale:
`build/startup-native/`, insbesondere `linux-startup-provenance.json`,
`crosscheck.json` und `native-ci.json`.

| Startobjekte | Installiertes Binärpaket | Quellpaket / genaue Version |
| --- | --- | --- |
| Scrt1.o, crti.o, crtn.o | libc6-dev:amd64 | glibc / 2.39-0ubuntu8.9 |
| crtbeginS.o, crtendS.o | libgcc-13-dev:amd64 | gcc-13 / 13.3.0-6ubuntu2~24.04.1 |

Der tatsächlich installierte GCC-Copyrighttext benennt libgcc/crtstuff unter
GPLv3 oder später mit GCC Runtime Library Exception3.1 und enthält die
Ausnahme. Der glibc-Pakettext und die Source-Paketpatches bleiben mit den zuvor
gelesenen speziellen Startdatei-Regeln abzugleichen. Die erfassten Dateihashes
bezeichnen den nach dem Linken geprüften installierten Bestand; die Inventur
beweist keine unveränderliche Historie jeder SDK-Datei seit deren Installation.

### glibc-Quellpaket und besondere Startdatei-Regeln

Die passende [Ubuntu-Quellenbeschreibung](https://archive.ubuntu.com/ubuntu/pool/main/g/glibc/glibc_2.39-0ubuntu8.9.dsc)
nennt genau die beobachtete Version2.39-0ubuntu8.9. Original- und Debian-
Quellarchive wurden von derselben offiziellen Quelle geladen und gegen deren
veröffentlichte SHA256 geprüft. Die drei Originale `start.S`, `crti.S`, `crtn.S`
unter sysdeps/x86_64 enthalten die besondere Erlaubnis, ihre kompilierten
Fassungen mit anderen Programmen zu verbinden und diese ohne Beschränkung aus
der Verwendung dieser Dateien zu verteilen. Die übrigen LGPL-Regeln bleiben
laut Original für andere Verwendungsarten bestehen.

Alle116 Einträge der im Debian-Archiv enthaltenen Quilt-Serie wurden gelesen;
kein Patch erwähnt diese drei Pfade. Das stärkt den versionsbezogenen
Quellenabgleich, ist aber keine reproduzierte Übersetzung der Objektdateien
und keine vollständige Prüfung aller Debian-Buildskripte. Die PGP-Signatur der
Quellenbeschreibung wurde nicht geprüft; nachgewiesen sind offizieller
HTTPS-Abruf und Übereinstimmung der Archivbytes mit den publizierten Hashes.
Originale und Dateihashes: `build/glibc-source-review/source-review.json`,
Quellenbeschreibung, beide Archive und die drei gelesenen Quelldateien.

## Geladene Windows-Runtime-Dateien

Der optionale Karten-Buildweg aktiviert unter MSVC zusätzlich `/VERBOSE`.
Der vorhandene Build-Wrapper bewahrt vollständige Ausgaben in
`checked-build.log`. Grundlage, am 12. Oktober gelesen:
[Microsoft /VERBOSE](https://learn.microsoft.com/en-us/cpp/build/reference/verbose-print-progress-messages?view=msvc-170),
insbesondere Bibliotheks-/Objektpfade und aufgelöste Symbole.

`tools/record_windows_runtime.py` bindet die benannten CRT-/uuid-Mitglieder der
Karten an tatsächlich als geladen gemeldete Bibliotheken. Eine Suchzeile allein
ist kein Ladungsnachweis. Relative Namen benötigen genau einen vollständigen
Suchpfad; mehrdeutige gleichnamige Dateien werden abgewiesen. Jedes benannte
Objektmitglied muss im abgeschlossenen gemeinsamen Build-Log enthalten sein.
App/CLI bleiben durch ihre Kartenidentitäten gebunden, SDK-Dateihashes werden
nach dem Build erfasst und erneut geprüft. Die Bibliotheken werden weder in den
Bericht kopiert noch durch den Prüfer ausgeführt. Innere AccessKit-DLL-Beiträge
sind durch diesen begrenzten App-/CLI-Prüfweg noch nicht zugeordnet.

Vier neue lokale Verträge bestehen: Such-/Ladungsunterschied, eindeutige
vollständige Pfade, mehrdeutige SDKs und richtige Kartenmitglied-Auswahl.
Der Parser liest beide tatsächlich heruntergeladenen MSVC-Karten mit den oben
genannten Mitgliederzahlen. Zwei CTest-Gruppen und die Python-Gesamtprüfung mit
72 Tests (eine plattformspezifisch übersprungen) bestehen. Neue native Windows-
Ausführung des erweiterten Build-/Logwegs folgt. Originale:
`build/windows-runtime-inventory-check/`. Kein neuer Installationsstand.

Die erste native Ausführung zu439cecb scheitert in beiden Windows-Konfigurationen
im neuen Inventurprüfer, noch vor der UI-Gesamtprüfung. Das gesicherte Debug-
Artefakt11725203477 stimmt mit seinem GitHub-SHA256-Digest überein und enthält
nur Bibliothekssuchzeilen, keine Loaded-/Found-/Referenced-Zeilen. Der Prüfer
verweigert deshalb einen Ladungsnachweis; vorhandene Karten/Paketabnahmen der
Vorversion werden dadurch nicht rückwirkend ungültig. Quelle:
`build/windows-runtime-native/checked-build-debug.log`.

Für die Nachprüfung wird der Linker auf vollständiges `/VERBOSE` umgestellt;
bei Visual-Studio-Generatoren erhält MSBuild normale Ausführlichkeit, damit
gewöhnliche Werkzeugausgaben im Log bleiben. Der Prüfer akzeptiert weiterhin
keine Suchzeilen als Ladungsnachweis. Die lokale Python-Gesamtprüfung bleibt
mit72 Tests (eine übersprungen) erfolgreich. Neue native Nachprüfung folgt.

### Abgeschlossene Windows-Nachprüfung

[CI38202931306](https://github.com/Lulus792/SecondBrain/actions/runs/38202931306)
zua490e5c besteht unter Windows/Linux in Debug/Release mit jeweils92 Tests und
beiden Release-Paketen; Mac-Jobs warten auf Runner. Die Windows-Inventur wurde
in beiden Konfigurationen erfolgreich ausgeführt. Der vollständige Release-
Build-Log enthält1411 Loaded-Zeilen. Die erste Verweigerung ist damit im
beschriebenen Umfang durch ausreichende native Linker-Ausgabe behoben.

Der heruntergeladene Windows-Release-Nachweis11726008467 und das Paket11725569084
stimmen mit ihren GitHub-SHA256-Digests überein. Der Bericht passt zu den beiden
tatsächlichen Paketprogrammen; sein Loghash passt zum enthaltenen Build-Log.
Die rohe CTest-Datei enthält92 Prüfblöcke und92 Pass-Marker, keinen Fail-Marker.
Ihre gemischte Windows-Textkodierung wurde für diesen Nachweis nicht verlusthaft
in UTF-8 umgedeutet. Originale: `build/windows-runtime-native/`, insbesondere
`crosscheck.json`, `windows-runtime-provenance.json`, die beiden Logs und ZIPs.

Der native Release-Bericht nennt vier tatsächlich geladene, nach dem Build
gehashte Dateien:

| Bibliothek | Tatsächlicher Herkunftspfad |
| --- | --- |
| libcmt.lib, libvcruntime.lib | Visual Studio18/Enterprise, MSVC14.51.36231/lib/x64 |
| libucrt.lib | Windows Kits10/Lib/10.0.26100.0/ucrt/x64 |
| Uuid.Lib | Windows Kits10/Lib/10.0.26100.0/um/x64 |

Die vollständigen Pfade und individuellen SHA256 stehen im Bericht. Die
Zuordnung benennt App-/CLI-Runtime-Dateien, nicht automatisch alle internen
Beiträge der AccessKit-DLL. Versionsbezogene SDK-Verteilungsbedingungen und
deren Endproduktdarstellung bleiben mit den Originalquellen abzugleichen.
Die installierte App bleibt0.9.74/eabf5b3ff114; kein neuer menschlicher Nachweis.

## Innere Windows-AccessKit-DLL

Der optionale Karten-Buildweg fordert nun auch eine MSVC-Karte für die tatsächlich
erzeugte AccessKit-DLL an. Dafür wird `cargo rustc --lib` mit einem zusätzlichen
abschließenden `-C link-arg=/MAP:...` verwendet. Die vorhandene Lockdatei,
Release-Konfiguration, explizite MSVC-Zielauswahl und crt-static-Policy bleiben
Teil desselben Buildbefehls. Die Karte wird wie bei App/CLI über eine Identität
mit DLL- und Karten-SHA256 verbunden und als Entwicklungsnachweis aufbewahrt.
Im normalen Build ohne Kartenoption bleibt der vorhandene Cargo-build-Aufruf.

Grundlage, am 12. Oktober gelesen:
[Cargo rustc](https://doc.rust-lang.org/cargo/commands/cargo-rustc.html),
insbesondere Argumente ausschließlich für den abschließenden gewählten Target
und die erforderliche einzelne Target-Auswahl. Die Argumente werden nicht als
globale Flags an Abhängigkeiten oder Host-Buildskripte weitergereicht.

Lokal besteht die Python-Gesamtprüfung mit72 Tests (eine übersprungen). Eine
tatsächlich kompilierte kleine macOS-Prüfbibliothek mit cdylib/staticlib zeigt,
dass derselbe Cargo--lib-Vertrag die beiden Ausgaben, die zusätzliche Linkerkarte
und die DLL-/Kartenhashbindung erzeugt, einschließlich Pfad mit Leerzeichen/Umlaut.
Das ist eine Cargo-Orchestrierungsprüfung auf Intel/macOS, keine Windows-
AccessKit-Ausführung. Originale: `build/accesskit-map-check/`.
Die neue native Windows-DLL-Prüfung folgt; die inneren Beiträge werden erst
nach ihrer tatsächlichen Karte und dem Paketvergleich als zugeordnet behandelt.
