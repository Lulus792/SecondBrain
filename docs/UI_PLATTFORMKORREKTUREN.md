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
Der lokale Gesamtlauf zu fe7d18e besteht mit 80/80 Prüfungen (487,70 s).
Die anschließende native CI bestätigt den Linux-Resize-Fall; dort besteht
79/80, nur die Textstilprüfung scheitert noch an einem Wert. Windows Release
besteht 77/79: native Stilabfrage und ein Rohbyte-Vergleich der Editorläufe
scheitern. Beide UI-Builds in Debug/Release bleiben erfolgreich. Originale:
`ci-jobs-3.json`, `ci-failure-114538327335.log`, `ci-failure-114538327369.log`.

Die Stilprüfer wählen jetzt den Dokumentbereich: Windows sucht unter allen
Namensübereinstimmungen einen Document-/Edit-Bereich; Linux überspringt die
namensgleiche Überschrift. Erwartete Stilwerte bleiben unverändert, Fehler
nennen zusätzlich Wort, tatsächlichen und erwarteten Wert.
Microsoft beschreibt für den [Document-Control-Typ](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportdocumentcontroltype)
den Dokumentcontainer und sein erforderliches Text-Pattern; Original am
11. Oktober gelesen. Die zusätzliche Abgrenzung bei gleichen Namen ist
eine eigene Prüferkorrektur, kein Ersatz für den nativen Nachweis.
Der native Lauf zu a6d6f09 besteht auf Windows Release vollständig einschließlich
Paketabnahme (Job 114541692685). Windows Debug scheitert noch an zwei Prüfungen
vor abgeschlossener Reader-Vorbereitung. Linux findet bei „Stilprüfung“ weiterhin
den Kartenkopf mit `400|0|Noto Sans|15.0` statt der Dokumentüberschrift.
Die weitere Linux-Abgrenzung erlaubt nur Dokument-/Eingabebereiche; die
[AT-SPI-Rollen](https://gnome.pages.gitlab.gnome.org/at-spi2-core/libatspi/enum.Role.html)
wurden am 11. Oktober gelesen. Namensgleiche Textläufe reichen nicht als Scope.
Die Struktur-/Tastaturprüfer warten begrenzt auf den fertigen Reader, bevor sie
vollständige Überschriften beziehungsweise den letzten Link erwarten. Lokal
bestehen beide danach (55,35 s); neue native Nachprüfung folgt. Originale:
`ci-final-jobs-3.json`, `ci-final-linux-failure.log`,
`ci-final-windows-debug-failure.log` und `ready-probes.log`.
Der Editorprüfer vergleicht alle veröffentlichten Felder und verwendeten
Arraywerte statt undefinierter C-Padding-Bytes. Die Auswahl-/Geometrieprüfung
bleibt exakt und erkennt absichtlich veränderte Exporte. Die gezielten lokalen
Prüfungen bestehen (15,76 s, danach erweiterter Editorprüfer 2,11 s); die neue
native CI-Nachprüfung folgt. Das sind Prüferänderungen, keine Behauptung einer
abgeschlossenen assistiven oder allgemeinen Windows-/Linux-Abnahme.
Im [CI-Lauf 38162552488](https://github.com/Lulus792/SecondBrain/actions/runs/38162552488)
zu fe7d18e bestehen inzwischen die tatsächlichen Windows-UI-Builds mit MSVC
in Debug und Release sowie die Linux-UI-Builds mit GCC. Die native UI- und
Paketprüfung lief bei der Abfrage noch. Original: `ci-jobs-2.json` in der Verifikation.
Installiert bleibt zunächst 0.9.59. Leistung, frühe Scrollbefehle, tatsächliche
IME-/Screenreader-/Gerätebedienung und die übrige [Release-Liste](RELEASE.md)
bleiben offen. 1.0 nur nach ausdrücklicher Nutzerfreigabe.
