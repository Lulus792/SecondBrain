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

Die neue native Windows-/Linux-Ausführung dieses optionalen Buildwegs folgt.
Sie ist getrennt von der abgeschlossenen 0.9.74-Produktprüfung zu eabf5b3.
Systemruntimes, SDK-/Toolchainbedingungen und innere Rust-Anteile sind weiterhin
gezielt abzugleichen; dies ist keine vollständige Lizenzfreigabe. Die installierte
App bleibt die bereits geprüfte saubere 0.9.74/eabf5b3ff114.
