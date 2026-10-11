# Scrollen während der Dokumentvorbereitung

Stand: 11. Oktober 2026. Umsetzung ab 0.9.61.

## Bestätigter Fehler

Ein Scrollbefehl während der Vorbereitung kam im richtigen Reader-Slot an,
wurde aber am Frame-Ende verworfen: Die Ladezeile markierte den Lesebereich
nicht als benutzt. Der erste produktive Ereignistest mit Bild ab bestätigt den
Verlust (`build/reader-scroll-verification/before.log`, 0,54 s). Die vorherige
Paketprüfung hatte dieselbe Timing-Grenze bei einem langen Lizenztext gefunden.

## Verhalten

Der ladende Reader behält ausstehende Scrollbefehle. Sobald das passende
Dokument fertig ist, wird zuerst seine tatsächliche Inhaltshöhe gemessen.
Anschließend verarbeitet der vorhandene Scrollweg die Eingaben und begrenzt sie
auf diese Höhe. Eine große Eingabe zeichnet dadurch kein Bild jenseits des Endes.
Richtungswechsel und die vorhandene kurze Bewegung bleiben erhalten. Es gibt
keinen künstlichen Mindesttimer und keinen zusätzlichen Worker.

Dokument-/Projektgenerationen und vorhandene Fokus-/Aufgabengrenzen bleiben
maßgeblich: Alte Eingaben dürfen nicht auf die nächste Notiz übergehen. Die
Korrektur betrifft Hauptreader und modale Kontext-/Lizenzreader. Editor, Suche
und Formularscrollen behalten ihre bisherigen Wege.

## Grundlage

Apple [Loading](https://developer.apple.com/design/human-interface-guidelines/loading)
empfiehlt sichtbare Rückmeldung und weitere benutzbare Aktionen beim Laden.
[Scroll views](https://developer.apple.com/design/human-interface-guidelines/scroll-views)
beschreibt erwartbares Scrollen, Kontext und elastische Indikatoren. Die am
11. Oktober titelgeprüft eingesehenen Originale wurden vor diesem Schritt erneut
gelesen; sie liegen im UI-Bericht vom 18:36:07 unter `sources/`.
Das Halten von Reader-Eingaben und die Messung vor ihrer Anwendung sind eigene
Übertragung, kein behaupteter Apple-Algorithmus.

## Prüfungen und Grenzen

[test_reader_scroll.c](../tests/test_reader_scroll.c) prüft echte SDL-Tasten und
Mausradereignisse im produktiven Desktop: wiederholte Eingaben, Gegenrichtung,
Übernahme nach Bereitschaft, getrennten Kontextreader, Verwerfen bei Notizwechsel,
sehr große Eingaben und Bewegung ohne reduzierte Animation. Layoutbilder halten
die Worker-Race beobachtbar; tatsächliche Reader-/Kontextbilder wurden zusätzlich
gerendert und betrachtet. Es werden weder Worker-Verzögerung noch ein anderer
Produktalgorithmus für den Test eingeschaltet.

45 neue Prüfbedingungen bestehen; zusammen mit Reader-Vorbereitung,
Hilfe-Rückkehr und Interaktion bestehen vier gezielte Läufe (36,05 s).
45 Scroll-, 117 Reader- und 622 native Mac-Prüfbedingungen bestehen auch unter
haltendem ASan/UBSan: 39 eigene C-Dateien, 56 aktuelle Quell-/Include-Hashes
vor/nach dem Lauf geprüft. Externe UI nicht instrumentiert, kein Leak-/
ThreadSanitizer-Nachweis. Originale: `build/reader-scroll-verification/`,
`build/reader-scroll-sanitizers/` und `build/current-ui/reader-scroll-69864/`.

Gesamt-, Paket- und neue native Zielplattformabnahme folgen. Installiert bleibt
zunächst 0.9.60. Dies behebt den bestätigten Eingabeverlust, nicht die gesamte
Leistungsaufgabe: erste Quell-/Parser-/Rasterarbeit, große echte Wissensbasen,
reale Geräte/Monitore/Trackpads, IME und Screenreader bleiben offene Abnahmen.
Die übrige [Release-Liste](RELEASE.md) und die ausdrückliche 1.0-Freigabe bleiben.
