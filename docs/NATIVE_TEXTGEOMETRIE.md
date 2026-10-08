# Native Zeichenrechtecke im Editor

Stand: 8. Oktober 2026, Umsetzung ab 0.9.42.

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

Die neue genaue Geometrie gilt für den Notizeditor. Leseansicht und andere
Eingabefelder benötigen weitere Integration. Wiederverwendung der exportierten
Geometriemetadaten bei unverändertem Layout und die Leistung großer realer
Editorinhalte sind weitere Aufgaben. Vollständige Plattform-, IME- und
Screenreader-Abnahmen bleiben offen. Ausgeführte Nachweise stehen in
[STATUS](STATUS.md).

Primärquellen: der festgelegte [AccessKit-Koordinatenvertrag](https://github.com/AccessKit/accesskit/blob/ce8164ba92995cfa86005b6259115e08c8244253/accesskit/src/lib.rs)
und die lokale Quelle `accesskit_macos-0.27.1/src/util.rs`, am 8. Oktober gelesen.
Die macOS-Umrechnung verwendet `NSWindow.backingScaleFactor` und die
Fenster-/Bildschirmtransformation. Eigene Editorgrundlage:
[EDITORGEOMETRIE](EDITORGEOMETRIE.md).
