# Dokumentwechsel, Suche und Texteingabe

Stand: 7. Oktober 2026, ab 0.9.19. Neues Nutzerfeedback konkretisiert lange
Dokumentwechsel, Startfokus, Textcursor, Suchende und Anlegekarten.

## Lange Dokumente

Textmessungen benutzen begrenzte Caches mit vollständigem Bytevergleich bei
Hashkollisionen. Absatzlayouts speichern denselben Umbruchplan, der zuvor für
Höhe, Positionen, Schriftläufe und Ausrichtung frisch berechnet wurde. Inhalt,
Stilbereiche, Breite, Padding und Schrift gehören zum Schlüssel. Schrift- und
Pixeldichtewechsel verwerfen alle Pläne vor dem Freigeben der Schriften.
Der Layoutcache ist auf 32 MiB begrenzt. Bei fehlenden Ressourcen und sehr langen
Einzelabsätzen benutzt die App den vorhandenen Reflow; Originaltext bleibt erhalten.
Unsichtbare Zeichnung entfällt, die vollständigen nativen Dokumentblöcke bleiben.

Unveränderte Notizinventur und geladene Dateirevision ersetzen beim Wechsel nicht
den Graph. Speichern, geänderte Revisionen, neue Inventur, Projektwechsel und
fehlgeschlagene vorherige Aktualisierungen bauen ihn erneut auf. Der
[letzte vollständige Stand](STERNKARTEN_BESTAND.md) bleibt bei Fehlern erhalten.
Ohne Dateiwächter werden andere externe Änderungen erst durch einen vorgesehenen
Neuaufbau erfasst. Das ist keine vollständige Synchronisationsfunktion.

Lokale Metal-Messung auf macOS 14.6.1/Intel i7-8559U, 1336×840, derselbe
Journaleintrag und je 60 warme Stichproben: Layout im Ruhezustand zuvor 94,82 ms,
nach den Caches 8,62 ms im Mittel. Median des vollständigen Frames 96,44 → 10,61 ms.
Während Scroll-/Kamerabewegung bleiben lokale Renderzeiten um 28 ms; keine
allgemeine feste Bildrate. Temperatur, Last und andere Hardware begrenzen den
Vergleich. Erstmaliger Reflow und tatsächliche Langzeitabnahme bleiben relevant.

## Fokus, Cursor und Suche

Nach dem Laden eines Projekts liegt der Startfokus auf der Sternkarte. Pfeile
funktionieren unmittelbar. Tab/F6, Editieren und modal geschützte Eingaben behalten
ihre eigenen Wege. Editierbare Felder zeigen den nativen SDL-Textzeiger. Einfügen
benutzt einen dünnen blinkenden Caret zwischen Zeichen; neue Eingaben und explizite
Fokuswechsel zeigen ihn wieder. Überschreiben bleibt erkennbar. Präzise visuelle
Bidi-/IME-/native Textgeometrie bleibt eine eigene Release-Aufgabe.

Suche merkt sich vor dem Öffnen die Listenansicht und die große Leseansicht.
Escape, Leerung, Dokumentwahl und Verlassen von Suche/Ergebnisliste beenden sie.
Eine dadurch geöffnete Liste schließt; eine bereits offene Liste bleibt erhalten.
Ergebnisnavigation per Tab beendet die Suche nicht vor der Auswahl. Die Symbole
teilen gleich breite Plätze und skalierende Innenränder; das X bleibt am rechten
Ende mit passendem Abstand.

## Anlegekarten

Notizen: Titel, direkt wählbarer Wissensbereich, Dateiname und dessen Hilfetext.
Projekte: Name, Ordnername und optional verknüpfter Ordner. Felder haben gemeinsame
Breiten und getrennte Gruppen; Hilfetext erhält seine tatsächlich gemessene Höhe.
Abbrechen und Anlegen stehen mit passenden Breiten am rechten Abschluss. Kopf
und Abschluss bleiben bei großer Schrift erreichbar; der Körper darf scrollen.

Vor Umsetzung erneut gelesen: Apple [Text fields](https://developer.apple.com/design/human-interface-guidelines/text-fields),
[Text views](https://developer.apple.com/design/human-interface-guidelines/text-views) und
[Focus and selection](https://developer.apple.com/design/human-interface-guidelines/focus-and-selection).
Fokusstart, Cachetechnik, Cursorzeiten und konkrete Abstände sind eigene Übertragung.
Prüfungen und Grenzen: [STATUS](STATUS.md), verbleibende Arbeiten: [RELEASE](RELEASE.md).
