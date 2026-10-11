# Native Zeichenrechtecke in Eingabefeldern

Stand: 11. Oktober 2026, Editor ab 0.9.42, Wiederverwendung ab 0.9.43,
Suche und Formulare ab 0.9.49.

Der Notizeditor veröffentlicht Zeichenpositionen und Breiten aus dem tatsächlich
gezeichneten Glyphen-/Cursorplan. Schriftmessung wird dafür nicht ein zweites Mal
ausgeführt. Die Zeilenhöhe und der Ursprung stammen aus der gezeichneten
Eingabefläche; horizontale und vertikale Scrollposition werden abgezogen.
Außerhalb des sichtbaren Ausschnitts bleiben Quellenzeichen für native
Textnavigation erhalten. Die Rechtecke bezeichnen auswählbare Zeichenbereiche,
nicht die engsten Konturen einzelner gerasterter Glyphen.

Normale Grapheme, CRLF und ein leerer Schlussabsatz behalten die Einheiten des
[nativen Textvertrags](NATIVE_TEXTLAEUFE.md). Bidi-Level, Zeilen- und Stilwechsel
trennen Läufe. Nicht benachbarte visuelle Zeichenflächen erhalten ebenfalls
getrennte Läufe, damit deren Positionen in Schreibrichtung monoton bleiben.
Läufe von rechts nach links veröffentlichen Abstände vom rechten Rand; Läufe
von links nach rechts verwenden den linken Rand. Die Quellenreihenfolge wird
nicht umsortiert.

## Koordinaten und Lebensdauer

UI-Flächen und Zeichenrechtecke verwenden SDL-Fensterkoordinaten. Am nativen
Fensterknoten wird auf macOS und Windows die SDL-Pixeldichte als Transformation
angewendet. AccessKit erwartet dort physische Clientpixel. Eine Änderung der
Dichte invalidiert die veröffentlichte Momentaufnahme. Der bestehende Unix-Weg
mit seinen gemeldeten Fenstergrenzen bleibt unverändert; die reale HiDPI-Abnahme
unter X11/Wayland steht gesondert aus.

Die Übergabe kopiert den Plan, bevor seine kurzlebige UI-Ausgabe freigegeben
wird. Geometrieänderungen behalten Quellenkennungen und Auswahlpositionen bei.
Bei aktiver vorläufiger IME-Komposition werden keine Rechtecke der visuellen
Komposition an den gespeicherten Quelltext gehängt. Dann bleibt die bisherige
native Metadatenabbildung erhalten. Ein Graphem über 255 UTF-8-Bytes benötigt
weiterhin skalare native Teilstücke; hierfür wird keine unzutreffende genaue
Geometrie veröffentlicht.

## Wiederverwendung unveränderter Metadaten

Ab 0.9.43 hält die UI höchstens einen nativen Editorplan mit einem eigenen
Speicherbudget von 32 MiB. Die Grenze zählt Cacheobjekt, vollständige Quelle
und tatsächlich behaltene Laufkapazität. Der Glyphenplancache hat weiterhin
sein separates Budget. Größere Exporte bleiben vollständig, werden aber nicht
zusätzlich für Folgebilder behalten. Eine fehlgeschlagene Cacheallokation
verwirft keine bereits vollständig vorbereitete Ausgabe.

Der Schlüssel umfasst die exakten Quellenbytes, Schriftidentität, Zeilenhöhe,
Pixeldichte und ein-/mehrzeiligen Eingabemodus. Ein Hash dient nur zur schnellen
Vorauswahl; ein vollständiger Bytevergleich bestätigt die Quelle. Reine
Cursor-, Auswahl- und Scrolländerungen segmentieren Unicode und Wörter nicht
erneut. Der gespeicherte Plan verwendet einen Ursprung bei null; jede Ausgabe
erhält die aktuelle gezeichnete Feld-/Scrollposition. Die vertikale Berechnung
behält die Rechenreihenfolge des Renderers bei.

Ausgaben besitzen ihre Laufarrays selbst. Veränderung oder Freigabe einer
Ausgabe verändert den Cache nicht. Auch die Verdrängung eines Glyphenplans
durch andere Felder lässt diese Ausgaben gültig. Schriftneuladen und das
Leeren des Editorcaches entfernen die native Wiederverwendung und den zuletzt
gezeichneten Feldverweis. Während IME-Komposition bleibt der ursprüngliche
Quellenvertrag unverändert.

## Prüfungen und weitere Arbeit

Ein unabhängig vorbereiteter Schrift-/Bidi-/Cursorplan prüft Zeichenpositionen,
Breiten und Richtung bei Latein, Akzenten, Ligaturen, Hebräisch, Arabisch und
Emoji mit 100, 150 und 200 Prozent Schriftgröße. Eine mehrzeilige Scrollprüfung
erhält Quellenabdeckung, CRLF, leere Absätze und Richtung; sämtliche Rechtecke
verschieben sich um die tatsächlichen Scrollwerte.

Der echte macOS-Provider liefert für einen UTF-16-Zeichenbereich nach einem
Familien-Emoji die erwartete Breite, Höhe und Bildschirmposition. Der Test
prüft zusätzlich den nativen Editorrahmen. Er fand zunächst einen Faktor-zwei-
Fehler auf dem Retina-Display; die Fenstertransformation korrigiert ihn.
Dies ersetzt keine menschliche VoiceOver- oder Vergrößerungsabnahme.

## Suche und Formularfelder

Suche, Projekt-/Notiztitel, Ordner-/Dateiname, verknüpfter Ordner, Arbeitsordner,
Sicherungspfad und Wiederherstellungskennung übernehmen ebenfalls den gerade
gezeichneten Quellplan. Nuklear verwendet für diese Widgets ein gemeinsames
kurzlebiges Editobjekt. Jedes Feld kopiert seine Quelle und Geometrie unmittelbar
nach dem Zeichnen in eine eigene Ausgabe; das nächste Feld verändert sie nicht.
Am Anfang des folgenden Frames und beim Beenden werden diese Ausgaben freigegeben.

Ein vollständig abgeschnittenes Feld erhält keine Geometrie des vorherigen
Widgets. Vorläufige IME-Komposition meldet weiterhin die Originalquelle ohne
unpassende Kompositionsrechtecke. Die Veröffentlichung vergleicht die vollständigen
Quellenbytes erneut: nach einer zwischenzeitlichen Änderung werden alte Rechtecke
nicht mit dem neuen Text kombiniert. Die kleinen Felder verdrängen den gehaltenen
32-MiB-Notizeditorplan nicht und behalten keine zusätzlichen dauerhaften Caches.

Der produktive Feldprüfer kontrolliert alle sieben Feldtypen, getrennte Quellen,
gemischte Schreibrichtungen, Grapheme/Emoji, leere Texte, 100/150/200 Prozent
Schriftgröße, horizontalen Scrollversatz, IME und den erhaltenen Editorcache.
Der macOS-Providerprüfer ergänzt einen UTF-16-Zeichenbereich nach Akzent und
Familien-Emoji im Notiztitel mit Breite, Höhe und Bildschirmposition.

Die genaue Geometrie gilt für die Eingabefelder. Die Leseansicht benötigt weitere
Integration. Native Auswahl-/Ersetzungsaktionen der einzeiligen Felder sind noch
nicht angebunden; die bisherige native Set-Value-Aktion bleibt erhalten.
Die Leistung großer realer
Editorinhalte, Exporte über der Cachegrenze und das erste lange Dokumentlayout
sind weitere Aufgaben. Vollständige Plattform-, IME- und
Screenreader-Abnahmen bleiben offen. Ausgeführte Nachweise stehen in
[STATUS](STATUS.md).

Primärquellen: der festgelegte [AccessKit-Koordinatenvertrag](https://github.com/AccessKit/accesskit/blob/ce8164ba92995cfa86005b6259115e08c8244253/accesskit/src/lib.rs)
und die lokale Quelle `accesskit_macos-0.27.1/src/util.rs`, am 8. Oktober gelesen.
Die macOS-Umrechnung verwendet `NSWindow.backingScaleFactor` und die
Fenster-/Bildschirmtransformation. Eigene Editorgrundlage:
[EDITORGEOMETRIE](EDITORGEOMETRIE.md).
