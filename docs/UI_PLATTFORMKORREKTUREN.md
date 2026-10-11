# C-Textcache und native Plattformprüfungen

Stand: 11. Oktober 2026. Korrekturen ab 0.9.60. Die Prüfungen auf weiteren
Zielplattformen werden erst nach ihren tatsächlichen Ergebnissen als bestanden
bezeichnet.

## Bestätigte Ausgangsbefunde

[CI-Lauf 38158192485](https://github.com/Lulus792/SecondBrain/actions/runs/38158192485)
zum Quellcommit 4d984a6 bestätigt drei Probleme:

- Windows: MSVC C2036 in `app/plain_native.inc:35`, Indexberechnung an `void *`.
- Linux: Reader-Resize-Prüfung scheitert direkt nach einer Größenanforderung.
- Linux: native Textstilabfrage auf „Stilprüfung“ findet keinen passenden Text.

Logauszüge stehen unter `build/help-return-verification/prior-*-failure.log`.
Eine lokale strenge C17-Prüfung bestätigt den ersten Fehler unabhängig von MSVC:
`build/platform-ui-verification/strict-before.log`.

## Typisierte Cache-Einträge

Die Suche nach dem ältesten Eintrag bindet das gehaltene Array vor der
Indexberechnung an `SBPlainPlan *`. Dadurch erfolgt die Adressierung mit der
tatsächlichen Größe eines Eintrags. Cachebudget, Quellenbindung und Eigentum
der Ausgabe bleiben erhalten. Die eigene UI-Bibliothek behandelt Pointer-
Arithmetik an `void *` auch mit GCC/Clang als Compilerfehler. Der eigene Code
bleibt C17; externe Bibliotheken bleiben UI-Abhängigkeiten.

Der neue [Cache-Prüfer](../tests/test_plain_native_cache.c) erzeugt zwölf echte
native Textausgaben mit je 60.000 Zeichen. Eine gehaltene Ausgabe bleibt dabei
vollständig; erneute Aufbereitung erhält Rechtecke, Positionen und Größen.
Das Ändern und Freigeben einer Ausgabe verändert die andere nicht. Dieser
Prüfer belegt keinen vorherigen Laufzeitabsturz; der negative Nachweis ist der
Compilerfehler. Die strenge C17-Prüfung besteht nach der Korrektur.

## Tatsächliche Fenstergröße

[SDL_SetWindowSize](https://wiki.libsdl.org/SDL3/SDL_SetWindowSize) fordert eine
Größe an; diese kann asynchron und unter Einschränkungen angewendet werden.
[SDL_SyncWindow](https://wiki.libsdl.org/SDL3/SDL_SyncWindow) wartet begrenzt auf
den endgültigen Fensterzustand. Beide Originale am 11. Oktober erneut gelesen.

Der Reader-Prüfer synchronisiert den Wechsel und prüft anschließend die
tatsächlichen Maße. Er fordert 780×900 an, da die Anwendung eine Mindestbreite
von 780 setzt. Die frühere Anforderung 750×900 war kein Nachweis dieser Größe.
Eine zusätzliche lokale Prüfung auf genau 750 bestätigte diese Ungenauigkeit;
die zulässige Größe besteht. Auch der Hilfe-Prüfer synchronisiert und prüft
seinen 780×520-Fall. Diese Änderungen betreffen die Prüfer, nicht die
Interaktionsgeschwindigkeit der normalen App.

## Textbereich beim nativen Linux-Client

Der AT-SPI-Prüfer wählte bisher das erste Objekt mit passendem Namen. Eine
Überschrift kann denselben Namen wie der Reader tragen. Für eine Textstilabfrage
wird jetzt zusätzlich der angeforderte Text im tatsächlich angebotenen Text-
Interface gesucht. Die anschließenden Font-/Stilwerte und Offsets bleiben
unverändert streng geprüft; fehlende passende Bereiche bleiben Fehler.

[Atspi.Text.get_text](https://gnome.pages.gitlab.gnome.org/at-spi2-core/libatspi/method.Text.get_text.html)
liefert Text aus dem angebotenen Interface; die Rückgabe gehört dem Aufrufer.
Original am 11. Oktober gelesen. Die konkrete Namens-/Textauswahl ist eine eigene
Prüferkorrektur und benötigt ihre native Linux-Nachprüfung. Der Zugriff über
echten Clientcache bleibt erhalten; keine Umgehung durch interne Providerdaten.

## Lokaler Teilnachweis

Nach Korrektur bestehen Reader-Resize, Hilfe-Rückkehr und Cache-Prüfung zusammen
(48,31 s). Die übrigen gezielten Text-/Titel-/Mac-Providerprüfungen bestanden
vorher; die dazwischen geänderten Zeilen betreffen die Fensterprüfer und
Compileroption. Quellen und Originale unter `build/platform-ui-verification/`.
525 Cache-, 117 Reader-, 93 Hilfe- und 621 native Mac-Prüfbedingungen bestehen
auch unter haltendem ASan/UBSan: 40 eigene C-Dateien, 57 aktuelle Quell-/Include-
Hashes vor und nach dem Lauf geprüft. Externe UI-Bibliotheken nicht instrumentiert;
kein Leak-/ThreadSanitizer-Nachweis. Originale: `build/platform-ui-sanitizers/`.
Gesamtprüfungen und neue native Zielplattformnachweise folgen.
Installiert bleibt zunächst 0.9.59. Leistung, frühe Scrollbefehle, tatsächliche
IME-/Screenreader-/Gerätebedienung und die übrige [Release-Liste](RELEASE.md)
bleiben offen. 1.0 nur nach ausdrücklicher Nutzerfreigabe.
