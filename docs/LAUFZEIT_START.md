# Laufzeitdiagnose mit dem normalen Renderer

Stand: 12. Oktober 2026, ab 0.9.73. Entwicklerprüfung für die tatsächliche App,
kein zusätzlicher Schritt in der normalen Nutzung.

Die bisherigen Paketabläufe verwenden einen Software-Renderer. Der normale
App-Start überlässt SDL die Rendererwahl und kann zusätzliche Bibliotheken laden.
Die festen Imports allein beschreiben deshalb nicht diesen gesamten Startweg.

`--runtime-probe NEUE_DATEI.json --workspace TESTORDNER` startet dieselbe App mit
normaler Rendererwahl, zeichnet und präsentiert vier Frames, schreibt einen
JSON-Bericht und beendet sich. Der Bericht nennt Buildkennung, tatsächlich
gewählten Renderer, Fenstersystem und beobachtete geladene Module.
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
