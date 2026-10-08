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

## Weitere Politur am 8. Oktober 2026 (0.9.28)

Die ausgehende Ansicht bleibt für den Kartenwechsel in einer GPU-Textur.
Der zuvor synchrone `SDL_RenderReadPixels`-Schritt entfällt dabei; gewöhnliche
Frames rendern weiterhin direkt. Der bestehende Pixelvergleich prüft das alte
Bild und zwölf schnell unterbrochene Wechsel. Fehlende Snapshot-Ressourcen
führen zum direkten Wechsel, niemals zu einer veralteten Karte.
Bewegung berücksichtigt auch langsamere Frames bis 250 ms; die vorherige
50-ms-Grenze konnte die Dauer unter Last verlängern.

Das Desktop-Modell setzt den Startfokus selbst. Auch nach erfolgreichem
Anlegen eines Projekts beziehungsweise Öffnen eines Arbeitsordners liegt er
auf der Sternkarte. Der dünne blinkende Einfügecursor und der native
Textzeiger sind in der aktuellen Navigation nochmals geprüft.

Ein Klick auf freie Fensterfläche außerhalb von Suchfeld und Ergebnisliste
beendet jetzt die Suche. Freie Fläche innerhalb der Ergebnisliste erhält sie.
Das Suchsymbol selbst fokussiert das Feld. Escape, Löschung, Trefferwahl und
Dokumentfokus behalten ihre Wege. Vorher geöffnete Listen bleiben bestehen;
allein durch Suche geöffnete Listen schließen wieder. Das X liegt mit
20 statt 24 skalierten Einheiten am rechten Rand, zentriert in seinem Platz.

In den Anlegekarten teilen Überschrift, Beschriftungen und Felder dieselbe
linke Linie; die Hauptaktion schließt rechts mit den Feldern ab. Die
Scrollspur bleibt frei. Körper und fest erreichbare Fußzeile sind getrennt. Schon die erste
Formgröße enthält gemessenen Hilfetext und sämtliche Feldabstände; dadurch
verschiebt ein nachfolgender Frame das erste Klickziel nicht. Eine alte
Erfolgsmeldung wird beim Beginn einer neuen Anlage entfernt.

Apple [Search fields](https://developer.apple.com/design/human-interface-guidelines/search-fields),
[Text fields](https://developer.apple.com/design/human-interface-guidelines/text-fields) und
[Motion](https://developer.apple.com/design/human-interface-guidelines/motion)
wurden vor diesen Änderungen erneut über die offiziellen DocC-Daten gelesen.
Die genannten Zeiten, Maße, Fokus- und Suchzustände sind eigene Umsetzung.

Die isolierte Metal-Messung wechselt Eingang → START.md → Übergabe vom
7. Oktober, jeweils sechs Wechsel in 60 warmen Stichproben bei 1336×840 auf
dem Intel-Mac. Mit alter beziehungsweise neuer Snapshottechnik liegt das
95. Perzentil der Frames bei 40,85 beziehungsweise 26,66 ms. Median
14,53 → 14,79 ms und Maximum 84,19 → 87,83 ms verbessern sich nicht.
Dies belegt die reduzierte Verzögerung eines konkreten Renderwegs; kein
Versprechen einer festen Bildrate oder allgemeiner Geräteabnahme.

## Gezielte Nachprüfung in 0.9.33

Am 8. Oktober wurden die bestehenden Korrekturen zum Nutzerfeedback erneut
lokal im macOS-Release-Build geprüft: 135 Bewegungs-/Layout-Assertions bestehen,
einschließlich Start-/Zwischenbildern, schnell unterbrochenen Übergängen,
Scrollspur sowie beiden Anlegekarten bei normaler und 200%-Schriftgröße.
Die aktuellen Screenshots der beiden Anlegekarten wurden betrachtet.

Die Navigationsprüfung umfasst jetzt zusätzlich alle vier Kombinationen aus
vorher offener/geschlossener Liste und normaler/großer Leseansicht. Suche leeren,
sofort erneut tippen und mit Escape beenden stellen jeweils den ursprünglichen
Zustand wieder her. Insgesamt bestehen 144 Navigations-Assertions und die
48 Vergleiche von gecachten und frisch berechneten Textbildern. Startfokus ohne
Mausklick, Textzeiger, blinkender Einfügecursor und Such-X bleiben mitgeprüft.
Das sind automatisierte SDL-Eingabe-/Renderprüfungen, keine menschliche
Bewertung des Bewegungsgefühls auf allen Geräten. Die installierte Intel-Mac-App
bleibt 0.9.33 / d48ee9bf7075; dieser Schritt erweitert nur Prüfungen und Nachweise.
