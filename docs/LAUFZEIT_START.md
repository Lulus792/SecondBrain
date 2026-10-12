# Laufzeitdiagnose mit dem normalen Renderer

Stand: 12. Oktober 2026, ab 0.9.73. Entwicklerprüfung für die tatsächliche App,
kein zusätzlicher Schritt in der normalen Nutzung.

Die bisherigen Paketabläufe verwenden einen Software-Renderer. Der normale
App-Start überlässt SDL die Rendererwahl und kann zusätzliche Bibliotheken laden.
Die festen Imports allein beschreiben deshalb nicht diesen gesamten Startweg.

`--runtime-probe NEUE_DATEI.json --workspace TESTORDNER` startet dieselbe App mit
normaler Rendererwahl, zeichnet und präsentiert vier Frames, schreibt einen
JSON-Bericht und beendet sich. Der Bericht nennt Buildkennung, tatsächlich
gewählten Renderer, Fenstersystem, Sichtbarkeit und beobachtete geladene Module.
Er benötigt einen expliziten Arbeitsordner. Einstellungen werden weder geladen
noch gespeichert; Kombinationen mit Projektwahl, Einstellungen, Snapshot oder
anderen Testmodi werden vor dem Start abgewiesen. Bestehende Berichte werden
über den eigenen C-Dateizugriff nicht ersetzt. Der Diagnosepuffer ist auf 8 MiB
begrenzt; Fehler werden mit Exit 1 gemeldet. Normale Bedienung bleibt im normalen
Eventloop. Der fachliche Kern erhält keine Bibliotheksabhängigkeit.

## Plattformspezifische Erfassung

- macOS: dyld-Add-Callback für vorhandene und während der Aufnahme hinzugefügte
  Images; `dladdr` ordnet Headeradressen ihren Namen zu. Ein eigener atomarer
  Schutz begrenzt die Aufnahme. Der registrierte Callback bleibt danach inaktiv;
  er besitzt keinen freigegebenen Berichtspuffer. Die nicht threadsichere
  Enumeration über `_dyld_image_count` wird vermieden.
- Windows: Toolhelp-Modulsnapshot des eigenen Prozesses, explizite Wide-APIs,
  UTF-8-Umwandlung und Schließen des Handles. `ERROR_BAD_LENGTH` wird bis zu achtmal
  wiederholt, danach als Fehler gemeldet. Potenziell am API-Limit gekürzte Namen
  werden nicht als vollständiger Bericht ausgegeben.
- Linux: `dl_iterate_phdr` liefert die Namen der geladenen Images. Der leere Name
  der Hauptdatei wird ausgelassen; der Prozessprüfer hält deren SHA256 separat fest.
  Virtuelle Module können einen Namen ohne Dateipfad haben.

Originale am 12. Oktober gelesen: [SDL Renderername](https://wiki.libsdl.org/SDL3/SDL_GetRendererName),
[Apple dyld](https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/dyld.3.html),
[Apple dladdr](https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/dladdr.3.html),
[Microsoft Toolhelp](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot)
und [Module32FirstW](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-module32firstw).
Die Linux-man-pages waren über den Webabruf nicht vollständig erreichbar;
eine native Linux-Prüfung bleibt erforderlich.

## Ausgeführte Prüfung und Grenzen

Lokal auf Intel/macOS 14.6.1: **Metal/Cocoa, 394 Module** beobachtet. Außerhalb
von `/System/Library` und `/usr/lib` erscheint nur die App selbst. 13 isolierte
Projektdateien bleiben unverändert, vorhandene Berichte erhalten und mehrdeutige
Argumente abgewiesen. Vier betroffene CTest-Gruppen bestehen in abschließend 6,59 s.

Drei normale Metal-Starts und der Versuch, einen bestehenden Bericht zu ersetzen,
bestehen mit ASan/UBSan. Dabei erscheinen 395 Module einschließlich der
Entwickler-Sanitizerlaufzeit. Eigener C-Code ist instrumentiert, externe UI nicht;
Leaks aus, kein TSan. 65 Quell-/Include-/generierte Headerhashes wurden abgeglichen.
Originale: `build/runtime72-verification/`, `native-probe.log`,
`probe-checked-regression.log`, `asan-final-probe/results.log` und
`build/runtime-probe-sanitizers/sources.json`.

Der neue Prozessprüfer läuft auch am entpackten Paket. CI speichert seine
`verification.json` einschließlich Programmhash. Aktuelle vollständige lokale,
native Windows-/Linux-, Paket- und Installationsprüfung von 0.9.73 stehen noch aus;
installiert ist weiterhin 0.9.72/c40c9fd.

Die Modulliste ist eine begrenzte Beobachtung beim Start. Spätere native Dialoge,
IME, Rendererwechsel, andere GPU-Treiber, nachgeladene Benutzerplugins und lange
Sitzungen sind dadurch nicht abgenommen. Beobachtete Systemmodule werden nicht
mitgeliefert und sind keine Zuordnung aller eingebetteten statischen Bestandteile.
Eine vollständige Lizenz-/SDK- oder Mindest-OS-Freigabe folgt daraus nicht.
Sie ergänzt [Paketimporte](PAKET_LAUFZEIT.md) und [Lizenzinventur](LIZENZ_INVENTUR.md).

## Gefundener Windows-Startfehler und Korrektur

Der neue normale Startweg scheitert in Windows Release zu 51b6963
([CI38193619573](https://github.com/Lulus792/SecondBrain/actions/runs/38193619573)):
88/89 Tests bestehen, nur die Startdiagnose bricht ab. AccessKit Windows 0.35.1
verlangt seinen Subclassing-Adapter vor dem ersten sichtbaren Fenster. Die App
zeigte bislang das Fenster bereits in `sb_ui_init`; erst danach wurde der Adapter
in `sb_desktop_init` angelegt. Versteckte Software-Testfenster verdeckten diesen
Fehler des normalen Starts. Der native Linux-Release samt Paket besteht dagegen.

Das offizielle Crate-Archiv wurde gegen den bereits festgelegten SHA256
`ce63f35d6bdcf59f26b76b3379063f738e6412cef46999cc772d46aa3de35adb`
geprüft und `src/subclass.rs` gelesen. Sein Sichtbarkeitscheck bestätigt die
Ursache. Original: [AccessKit Windows 0.35.1](https://static.crates.io/crates/accesskit_windows/accesskit_windows-0.35.1.crate).
Die docs.rs-Seite war nicht erreichbar; die Aussage stammt aus dem geprüften
Originalarchiv und dem tatsächlichen Jobfehler.

Der Desktop erstellt nun anfangs ein verborgenes Fenster bei unveränderter
normaler Rendererwahl, richtet Dialoge, Accessibility, Systemstil und Modell ein
und zeigt es anschließend an. Fehler beim Anzeigen führen zu geordnetem Abbau
und einer Fehlermeldung. Die eigenständige `sb_ui_init`-Anbindung behält ihr
bisheriges Verhalten. Die Diagnose verlangt jetzt ausdrücklich
`window_visible: true`; dauerhaft versteckte Fenster erfüllen die Prüfung nicht.

Vor der Korrektur bestehen lokal 89/89 Tests zu 51b6963 in 461,19 s; 211
Produktinputs und Binärhash unverändert. Dieser Lauf belegt nicht die neue
Reihenfolge. Nach der Korrektur bestehen fünf betroffene Gruppen einschließlich
normalem Start, nativer Mac-Anbindung, Erststart und Einstellungsfehlern in
25,05 s. Die neue Windows-Abnahme und der neue vollständige Lauf folgen.

Der SHA256-verifizierte ursprüngliche Linux-Release-Nachweis (Artefakt
11722576971, ZIP `45afbc65eeefbd0742b5fcfce8e852166a61e8e59a7ada4f2e3b83d9ad1a0d3c`)
bestätigt OpenGL/X11, vier Frames, 65 Module und 13 erhaltene Projektdateien im
entpackten Paket. Programmhash stimmt mit dem Importbericht überein. Alle
beobachteten Dateipfade liegen im Systembereich, hinzu kommt `linux-vdso.so.1`.
Keine reale GPU-/Wayland-/menschliche Bedienabnahme. Originale:
`build/runtime73-verification/`, insbesondere `linux-native-runtime.json`,
`window-order-tests.log` und `accesskit-subclass-original.rs`. Installiert bleibt
0.9.72; 0.9.73 wird erst nach der korrigierten Paketabnahme installiert.

Die korrigierte sichtbare Metal-Reihenfolge besteht außerdem drei Starts und
den bestehenden-Bericht-Fall unter ASan/UBSan; 65 aktuelle Quell-/Include-/Header-
Records abgeglichen, Headeroriginale vor dem nächsten sauberen Build gesichert.
Eigene C-Teile instrumentiert, externe UI nicht; Leaks aus, kein TSan. Originale:
`build/runtime73-verification/window-order-asan/` und
`build/runtime-probe-sanitizers/sources.json`. Das ersetzt keine positive
Windows-Ausführung der Korrektur.

## Konsolenzeilenenden im Prüfer

Lokal besteht die korrigierte Reihenfolge vollständig zu e66e023: 89/89 Tests
in 517,45 s, alle 211 Produktinputs und der Binärhash bleiben über den Lauf gleich.
Linux Release samt entpacktem Paket besteht erneut. Verifizierter Bericht:
OpenGL/X11, `window_visible: true`, 65 Module, 13 Projektdateien erhalten.
Artefakt 11722433364, ZIP-SHA256
`e3ad850e966ccb5fd87b3b0e4f97b92fb3429fe69ef64d6d12af94fdc0d33248`;
Programmhash stimmt mit dem Importbericht überein.

Die Windows-Debug-/Releasejobs der
[CI38194682964](https://github.com/Lulus792/SecondBrain/actions/runs/38194682964)
erreichen jetzt einen geschriebenen Diagnosebericht ohne den AccessKit-Absturz,
scheitern jedoch am bisherigen pauschalen Berichtvergleich (je 88/89).
`sb_build_info` enthält LF; `--version` schreibt über `fputs` in den normalen
MSVC-Textstream, der Bericht über `WriteFile` ohne Textübersetzung.
[Microsoft Text/Binary I/O](https://learn.microsoft.com/en-us/cpp/c-runtime-library/text-and-binary-mode-file-i-o?view=msvc-170),
am 12. Oktober gelesen, beschreibt den Standard-Textmodus von stdout.
Der Prüfer verglich die beiden Transportdarstellungen zuvor bytegetreu als Text.

Nur die Konsolen-Buildangabe wird nun CRLF→LF normalisiert. Andere Versionen,
ein verborgenes Fenster, fehlende Module oder weniger als vier Frames bleiben
Fehler. Die Validierung nennt einzelne fehlerhafte Felder; bei Buildabweichungen
auch beide Repräsentationen. CI speichert den Rohbericht bereits bei fehlgeschlagener
Validierung. Drei neue Vertragsprüfungen und ein erneuter tatsächlicher Metal-Start
bestehen; 13 Dateien erhalten. Der positive Windows-Nachweis dieser Prüferkorrektur
steht noch aus. Die tatsächlichen Programmdateien und Projektinhalte werden
für diesen Abgleich nicht normalisiert oder verändert.
Originale: `build/runtime73-fixed-verification/`, `full-ctest.log`,
`linux-native-runtime.json`, `probe-contract-final.log`, `probe-final-text-check.log`.

## Abschließende native, Paket- und Installationsabnahme

Installiert: **0.9.73 / 15e63b4f7960**, sauberer Intel/macOS-Release auf
macOS 14.6.1 / 23G93. 98 Dateien paketgleich. App-SHA256:
`72fb1cd672adc6b5bc8b8cc6601de1bb8d35e6e5aefeb01964cc4034a17d76b6`.
Die korrigierte Startreihenfolge besteht lokal **89/89 Tests zu e66e023**
(517,45 s), Binärhash und 211 Produktinputs unverändert. Danach ändern sich nur
Prüfer, CI-Nachweissicherung und Dokumentation: 211 Produktinputs bleiben gleich.
Die saubere 15e63b4-Fassung bestätigt beide Versionswege und normalen Start in
1,22 s. Ihre Buildkennung unterscheidet die Binärdateien; der vollständige Lauf
gehört ausdrücklich zu e66e023, Paket und Installation zu 15e63b4.

Das entpackte Paket besteht 126 Desktop-, 167 Tastatur- und 75 Sicherungsbedingungen,
Einstellungen in zwei Prozessen, CLI, Lizenzen, Versions- und Laufzeitprüfung.
Normaler Paketstart: Metal/Cocoa, sichtbares Fenster, vier Frames, 394 Module,
13 isolierte Projektdateien erhalten. Echter **72→73→72-Wechsel** erhält 20
Testdateien und Einstellungen; Sicherung/Wiederherstellung bestehen.
Drei Updateaufnahmen und die installierte Ansicht betrachtet. Beim Software-
Snapshot und dem zusätzlich geprüften normalen installierten Start bleiben alle
21 Gedächtnisdateien unverändert. App: minos 14.0, SDK 15.2; kein Nachweis auf
macOS 14.0/ARM64. [Startfehler und genaue Abdeckung](LAUFZEIT_START.md).

[CI38195529730](https://github.com/Lulus792/SecondBrain/actions/runs/38195529730)
zu 15e63b4: Windows/Linux Debug/Release vollständig erfolgreich, einschließlich
beider Release-Pakete. SHA256-verifizierte native Paketberichte bestätigen
Direct3D11/Windows mit 52 Modulen beziehungsweise OpenGL/X11 mit 65 Modulen;
jeweils sichtbares Fenster, vier Frames und unveränderte Testdaten.
Mac-CI wartet auf Runner. Server 2025/Xvfb ersetzen keine Windows-11-/Wayland-
oder menschliche Screenreaderabnahme. Originale unter
`build/runtime73-fixed-verification/`, `build/runtime73-verification/`
und `build/runtime73-update/`.

Die vorige App liegt als Rückfallkopie unter
`/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.72-20261012-043357`. Diese Abnahme schließt die bekannte Startreihenfolge
und den Textvergleich im dokumentierten Umfang ab; übrige Release-Anforderungen
bleiben offen. 1.0 ist nicht freigegeben.
