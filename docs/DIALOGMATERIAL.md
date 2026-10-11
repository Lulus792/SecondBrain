# Glasfläche vor dem Dialoginhalt

Stand: 11. Oktober 2026, Umsetzung ab 0.9.55. UI-Befund SBUI-077.

Der bisherige Dialoghintergrund ließ gedämpfte Texte anderer Fenster durch.
Die Glasfläche wurde bereits über der Sternszene berechnet; danach wurden aber
auch die Beschriftungen hinterliegender Fenster gezeichnet. Ein fast deckender
Farbhintergrund im Dialog beseitigte diese konkurrierenden Wörter nicht ganz.

## Komposition und Lebensdauer

Vor den eigenen Dialogtexten und Bedienelementen wird jetzt der passende Teil
der bereits berechneten Materialtextur eingesetzt. Die Sternszene, Lichtkanten,
Glasflächen und Darstellungseinstellungen bleiben erhalten. Hintergrund-UI
wird innerhalb der Dialogfläche verdeckt; der eigene Dialoginhalt liegt darüber.
Dies betrifft normale Dialoge und den Schutzdialog für ungespeicherte Änderungen.

Ein eigener Nuklear-Zeichenbefehl hält den UI-Besitzer und die aktuelle Rundungs-
geometrie. Die SDL-Textur wird erst bei der Umwandlung der Zeichenbefehle, nach
dem Raster-/Größenwechsel dieses Frames, aufgelöst. Es wird keine alte Texturadresse
im Befehl gespeichert. Die Methode unterstützt eine Vordergrundfläche je Frame.
Sie liest kein GPU-Bild auf die CPU zurück und erzeugt keine weitere Materialtextur.

Die Fläche besitzt eine deckende Innenseite und einen geglätteten Außenrand.
Außerhalb der Rundungen bleibt die Hintergrunddarstellung erhalten. Die
Texturkoordinaten folgen den tatsächlichen Fensterkoordinaten, auch nach Resize.
Fehlt die Materialtextur, enthält der Code eine deckende Ersatzfläche mit
passenden Hell-/Dunkel-/Kontrastfarben. Eine tatsächliche Speicherfehlerabnahme
dieses Ersatzpfads steht noch aus.

## Begrenzte Dreiecke im Software-Renderer

Die festgelegte SDL-Fassung prüft im Software-Dreieckspfad das Produkt aus
Quellkoordinate und Dreiecksfläche gegen den Bereich eines vorzeichenbehafteten
Integers. Große texturierte Dreiecke können deshalb ausfallen. Die Materialfläche
wird mit einem Mittelpunktsfächer und bei Bedarf unterteilten Dreiecken gezeichnet.
Der konservative Flächenwert berücksichtigt Ausgabepixel, Rundungsspielraum und
die begrenzten Texturabmessungen. Quellkoordinaten und Randdeckkraft werden bei
der Teilung interpoliert; Auflösung und Materialraster werden nicht abgesenkt.
Primärquelle: festgelegtes SDL3, `src/render/software/SDL_triangle.c`,
`SDL_SW_BlitTriangle`, Prüfung von `s2_x_area`.

Der Nuklear-Füllpfad verwendet die Punkte am Anfang seines Pfadpuffers.
Ein früherer Versuch mit dessen `path_offset` verschob die Punktfolge und
verletzte die Eckprüfung. Der korrigierte Zugriff folgt dem tatsächlich
festgelegten `nk_draw_list_path_fill`-Vertrag. Fehlgeschlagene Vorfassungen sind
keine Abnahmen; ihre Diagnoseprotokolle bleiben erhalten.

## Nachweise und Grenzen

352 unabhängige Pixelprüfungen vergleichen dieselbe Fläche mit und ohne
Hintergrundbeschriftungen. Die bisherige halbdeckende Komposition zeigt den
Unterschied; die neue Innenseite bleibt gleich. Eigener Vordergrundtext bleibt
sichtbar, ein Marker außerhalb der Rundung bleibt erhalten. Geprüft sind
800×600, 1336×840 und 1920×1080, 100/200% App-Schrift sowie hell/dunkel,
reduzierte Transparenz und Kontrastdarstellung auf dem Intel/Retina-Mac.
136 produktive Interaktions- und 621 native Mac-Provider-Assertions bestehen.
Neue Notiz und Aktionen wurden als tatsächliche Softwarebilder betrachtet.

Originale: `build/material-panel-verification/`, insbesondere
`targeted-corrected.log`, `new-note-corrected.png` und `actions-corrected.png`.
Native Renderer-, Sanitizer-, Gesamt- und Paketnachweise folgen in STATUS.
Reale Maus-/Trackpad-/Screenreaderbedienung, weitere Geräte und die aktuelle
Windows-/Linux-/ARM64-Abnahme bleiben offen. Der vollständige Release-Auftrag
ist damit nicht abgeschlossen; 1.0 wird erst nach Nutzerfreigabe gesetzt.

352/136/621 Assertions bestehen zusätzlich unter haltendem ASan/UBSan;
39 eigene C-Dateien instrumentiert und 54 Quell-/Include-Hashes geprüft.
UI-Abhängigkeiten sind nicht instrumentiert; kein Leak-/ThreadSanitizer-Nachweis.
Vier vor Present aufgenommene Metalbilder (Dichte 2,00) wurden angesehen, aus
48 erzeugten Aufnahmen. Der direkte Native-Versuch des Softwareprüfers scheiterte
an Snapshot-Pixelidentität; er wird nicht als bestandene Interaktionsprüfung
geführt. Nach Present erfasste alte/leere Frames zählen nicht als aktuelle Bilder.
78/78 lokale Release-Prüfungen bestehen (546,80 s). Paket/Installation folgen;
installiert bleibt 0.9.54.
