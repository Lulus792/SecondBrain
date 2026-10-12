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
Auch die tatsächliche lokale CMake-ExternalProject-Kette mit Cargo und
anschließendem Identitätsprüfer besteht, einschließlich Leerzeichen/Umlaut
in allen beteiligten Pfaden. Quelle: `build/accesskit-map-check/Orchestrierung ü/`.
Die anschließende native Windows-DLL-Prüfung ist im unten beschriebenen Umfang
abgeschlossen.

### Native DLL-Karte und Paketvergleich

[CI38204329310](https://github.com/Lulus792/SecondBrain/actions/runs/38204329310)
zu420f42c besteht unter Windows/Linux in Debug/Release, einschließlich beider
Release-Pakete. Die rohe Windows-CTest-Datei bestätigt jeweils92 bestandene
Prüfblöcke. Die Mac-Jobs warten weiterhin auf Runner.

Windows-Debug-Nachweis11726841419, Release-Nachweis11726483760 und
Release-Paket11726344057 wurden heruntergeladen und gegen ihre GitHub-SHA256-
Digests geprüft. Die Kartenhashes passen zu den jeweiligen Identitätsdateien.
Der Release-DLL-Hash passt zusätzlich exakt zur AccessKit-DLL im tatsächlichen
Anwendungsarchiv:

- DLL: `63e295847700376a39664d495702572c2bcfcbe96dfa67e79cb7a56317534392`.
- Karte: `b0f7889efeb097be4af88f2e1fc2647699436a1b196e06fdbc9603f3594783b7`.

Beide DLL-Karten benennen29 unterschiedliche libcmt-,21 libvcruntime- und99
libucrt-Objektmitglieder. Das belegt diese Runtime-Beiträge für die konkrete DLL.
Die Karte zerlegt das aggregierte Rust-Objekt nicht vollständig nach ursprünglichen
In-tree-Quellen; ein fehlender Einzelname beweist keine Abwesenheit eines Anteils.
Auch ein eigener Herkunftspfad-/Dateihashnachweis der SDK-Bibliotheken für diesen
Cargo-Linkvorgang fehlt noch. Der vorhandene App-/CLI-Bericht wird dafür nicht
als DLL-Bericht ausgegeben. Originalbytes und Abgleich:
`build/accesskit-map-native/`, `debug-review.json` und `release-review.json`.

### Direkte Eingabepfade für den Cargo-Linkvorgang

Der optionale Windows-Nachweis-Build fordert zusätzlich eine
`accesskit-inputs.rsp` über `/LINKREPROFULLPATHRSP` an. Laut der am12.Oktober
gelesenen [Microsoft-Dokumentation](https://learn.microsoft.com/en-us/cpp/build/reference/link-repro-full-path-rsp?view=msvc-170)
enthält diese Ausgabe die vollständigen Pfade der Linkereingaben, ohne die
Bibliotheksdateien wie `/LINKREPRO` zu kopieren. Die Option benötigt Visual Studio
17.11 beziehungsweise MSVC19.41 oder neuer; der Nachweis-Build weist ältere
Versionen ausdrücklich ab. Der gewöhnliche Build ohne Kartenoption bleibt
davon unberührt. Die dokumentierte MAX_PATH-Grenze gilt auch für die Ausgabedatei.

Der abschließende CMake-Prüfer verlangt eine vorhandene, nicht leere Pfadliste
und schreibt eine eigene Identität mit Programm- und Pfadlistenhash. Die
bisherige dreizeilige Kartenidentität bleibt kompatibel. CI bewahrt Liste und
Identität neben der Karte auf. Die Dateipfade sind zunächst Rohbelege; sie sind
noch keine vollständige Zuordnung aller Runtime-Anteile oder Lizenzfreigabe.

Sechs lokale CMake-Bedingungen bestehen, darunter fehlende/leere Listen,
Unicode-/Leerzeichenpfade, beide Hashes und unveränderter Kartenvertrag.
72 Python-Tests bestehen mit einer plattformspezifischen Überspringung.
Originale: `build/accesskit-map-native/Input ü check/` und
`input-path-python-tests.log`. Die tatsächliche neue Windows-Linkerausgabe und
der SDK-Dateiabgleich müssen anschließend nativ geprüft werden.

### Ausgeführter Eingabepfad-Nachweis

[CI38205225168](https://github.com/Lulus792/SecondBrain/actions/runs/38205225168)
zufeb45e8: Windows/Linux Release einschließlich entpackter Pakete erfolgreich.
Windows bestätigt92 Prüfblöcke und92 Pass-Marker in der rohen CTest-Datei;
Linux-Release-Log bestätigt92 Tests und22 aufgelöste Laufzeitdateien ohne
unaufgelöste/externe/konfligierende Einträge. Debug-Jobs liefen bei diesem
Abgleich noch, Mac-Jobs warteten auf Runner.

Windows-Nachweis11727029025 und Paket11727625582 sind gegen ihre GitHub-
SHA256-Digests geprüft. Die tatsächliche DLL im Paket hat den Hash
`ca06d2354abaa25ee0ca84f3981ee38dda1e72baf0f6f368ea148ca3b65e571e`.
Karten- und Eingabepfadidentität nennen genau diesen DLL-Hash; auch ihre
jeweiligen Dateihashes stimmen. Die Pfadliste ist tatsächlich UTF-16 LE mit
BOM und wurde entsprechend gelesen, nicht verlusthaft als UTF-8 interpretiert.

Sie nennt den vollständigen libcmt.lib-Pfad aus MSVC14.51.36231. Dieser passt
zum SDK-Dateibericht desselben abgeschlossenen Builds, SHA256
`d4cc4c224455825dec1669f9ad6a9872a4f5b37ed1daae1fd0b9044d7650d908`.
Die DLL-Karte benennt29 Mitglieder dieser Bibliothek. Damit sind für diesen
Teil DLL, benannte Mitglieder, direkter Herkunftspfad und nachträglicher
SDK-Dateihash verbunden.

Die tatsächliche Liste benennt sechs Rust-Archivinputs, darunter libstd und
libcompiler_builtins. Als Input übergebene Archive beweisen nicht, dass alle
ihre Mitglieder eingebaut sind. Der konservative Rust-Quellenabgleich bleibt
deshalb getrennt vom Nachweis einzelner gelinkter Beiträge.

Wichtige Grenze der beobachteten Ausgabe: Die über weitere eingebettete
Default-Library-Direktiven erreichten libvcruntime-/libucrt-Dateien fehlen in
dieser direkten Eingabepfadliste. Ihre benannten DLL-Beiträge sind durch die
Karte belegt, ihre eigenen DLL-Herkunftspfade werden durch diesen Schritt
noch nicht bewiesen. Ebenso sind nachträgliche SDK-Dateihashes keine unveränderliche
Aufzeichnung aller Dateiinhalte zum Linkzeitpunkt. Originale und Zuordnung:
`build/accesskit-map-native/inputs-release-review.json`, beide ZIPs,
`accesskit-inputs.rsp`, dessen Identität und `inputs-release-accesskit.map`.

### Gesicherte SDK-Eingabebytes im Nachweis-Build

Der optionale MSVC-Cargo-Linkvorgang verwendet ergänzend `/LINKREPRO` mit
einem eigenen bereits angelegten Ordner im Buildverzeichnis. Grundlage, gelesen
am12.Oktober: [Microsoft /LINKREPRO](https://learn.microsoft.com/en-us/cpp/build/reference/linkrepro?view=msvc-170)
und [Link-Repros](https://learn.microsoft.com/en-us/cpp/overview/how-to-report-a-problem-with-the-visual-cpp-toolset?view=msvc-170#link-repros).
Der Linker sichert dabei Eingabedateien für eine spätere Reproduktion. Der
gewöhnliche Build ohne Kartenoption verwendet diesen Weg weiterhin nicht.

`record_windows_runtime.py --accesskit-repro` bindet die DLL an ihre Karte,
fordert für jede benannte SDK-Bibliothek genau eine gesicherte Kopie und
vergleicht deren Bytes mit genau einer Datei der SDK-Inventur desselben Builds.
Fehlende oder mehrdeutige Kopien, andere Bytes sowie geänderte DLL-/Karten-/SDK-
Dateien werden abgewiesen. Der Bericht enthält nur Pfade, Hashes und benannte
Objektmitglieder. Die Repro-Bibliotheken sind weder Installationsbestandteil
noch Teil der ausgewählten CI-Uploads. Der Prüfer führt sie nicht aus und
sendet keinen Fehlerbericht an Microsoft.

Ein Bytevergleich belegt die SDK-Inhalte, nicht unabhängig den vom DLL-Linker
gewählten ursprünglichen physischen Pfad. Auch ersetzt er weder vollständige
Rust-Quellzerlegung noch den Abgleich der Verteilungsbedingungen.

Sechs neue lokale Verträge prüfen den erfolgreichen Bytevergleich sowie
fehlende, abweichende und mehrdeutige Kopien, geänderte SDK-Originale und
geänderte DLL-Identität. 78 Python-Tests bestehen mit einer plattformspezifischen
Überspringung; der vorhandene CTest-Inventurvertrag besteht in0,82s.
Log: `build/accesskit-map-native/repro-python-tests.log`.
Die neue tatsächliche Windows-Repro-Ausgabe und der Paketvergleich folgen.

### Debug-Nachprüfung des SDK-Vergleichs

Im Lauf38206412970 zu17e7a93 scheitert Windows Debug vor den UI-Tests beim
SDK-Bytevergleich. Der SHA256-geprüfte Nachweis11727567247 bestätigt die
Ursache: App/CLI verwenden libcmtd, libvcruntimed und libucrtd; die stets in
Release gebaute Cargo-DLL benennt libcmt, libvcruntime und libucrt. Der Prüfer
hatte nur Dateien berücksichtigt, die App/CLI selbst laden. Damit fehlte die
passende Release-SDK-Datei in seiner Vergleichsmenge, obwohl die DLL-Kopie
vorhanden war. Windows Release durchläuft diesen Vergleich bereits und lief
beim Abgleich noch in den UI-Tests.

Der Prüfer berücksichtigt jetzt die ausdrücklich benannten Release-/Debug-
Gegenstücke innerhalb derselben beobachteten SDK-Verzeichnisse. Er liest und
hasht dafür die tatsächlich angeforderte Bibliotheksdatei; eine Debug-Datei
wird niemals als Ersatz für eine fehlende Release-Datei akzeptiert, selbst
wenn beide in einer Prüffassung gleiche Bytes hätten. Der DLL-Kopienhash muss
weiterhin exakt passen. Die ursprüngliche App-/CLI-Ladungsinventur bleibt
getrennt von diesem zusätzlichen Vergleich.

Zwei neue lokale Verträge bestätigen unterschiedliche Debug-/Release-Bytes
und die Abweisung bei fehlendem Release-Gegenstück. 80 Python-Tests bestehen
mit einer Überspringung; CTest-Inventurvertrag besteht in0,13s. Originale:
`build/accesskit-map-native/repro-debug-failure.zip` und
`repro-debug-fix-tests.log`. Die neue native Debug-Nachprüfung steht noch aus.

### Bestätigter Release-Bytevergleich

Windows Release zu17e7a93 im Lauf38206412970 ist vollständig erfolgreich,
einschließlich92 Tests und entpacktem Paket. Nachweis11727976618 und Paket
11727463741 sind gegen ihre GitHub-SHA256-Digests geprüft. Der native Repro-
Bericht passt zur tatsächlichen Paket-DLL, SHA256
`af6e133f2a7f4cb9bc4709b6da174cda7fa69cefe943a1619d8792d1d0aa9945`,
und zur enthaltenen Kartenidentität. Die drei gesicherten Bibliotheken stimmen
bytegenau mit den SDK-Dateien überein: libcmt29, libvcruntime21 und libucrt99
benannte Objektmitglieder. Ihre Dateihashes sind die bereits oben dokumentierten
Hashes aus MSVC14.51.36231 beziehungsweise Windows SDK10.0.26100.0.

Die rohe CTest-Datei bestätigt92 Prüfblöcke und92 Pass-Marker. Weder das Paket
noch das heruntergeladene Nachweisarchiv enthält .lib/.rlib/.obj/.o-Dateien;
die SDK-Kopien bleiben im temporären Build. Originale:
`build/accesskit-map-native/repro-release-review.json`, beide ZIPs und
`repro-release-runtime-provenance.json`.

Die anschließende Nachprüfung zu24497c5 im Lauf38206935201 besteht unter
Windows/Linux in Debug/Release einschließlich beider Release-Pakete. Der
heruntergeladene Debug-Nachweis11727718400 stimmt mit seinem GitHub-SHA256
überein.92 rohe CTest-Prüfblöcke und92 Pass-Marker bestätigen den Lauf. Der
Bericht hält App-/CLI-Debug-Dateien und die drei tatsächlichen Release-SDK-
Bytevergleiche der DLL getrennt; DLL-/Kartenidentität passen zusammen. Die
Release-SDK-Dateihashes stimmen mit den oben dokumentierten drei Werten überein.
Original: `build/accesskit-map-native/repro-fixed-debug-review.json` und ZIP.
Die physische Ursprungspfadgrenze und der separate Lizenzbedingungen-Abgleich
bleiben bestehen; Mac-Jobs warten weiterhin auf Runner.
