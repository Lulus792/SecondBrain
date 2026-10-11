# Native Titel, Beschriftungen und Listenmarkierungen

Stand: 11. Oktober 2026, Umsetzung ab0.9.52.

Kartentitel, Dialogtitel, der leere Einstieg und die bestehenden einfachen
nativen Beschriftungen übernehmen die tatsächlich aufgezeichneten Textbefehle.
Sie enthalten die von Nuklear gewählte Position und den nach Kürzung noch
gezeichneten UTF-8-Text. Die Zeichnung bleibt unverändert.

## Sichtbarer Text und vollständiger Name

Der zugängliche Name behält die vollständige Bezeichnung. Der Textwert und seine
Zeichenrechtecke entsprechen dem sichtbaren Text, einschließlich Auslassungszeichen.
Es werden keine Flächen für abgeschnittene Originalbuchstaben erfunden. Dateien,
Notizinhalte und der gespeicherte Titel werden durch diese Abbildung nicht geändert.

Eine tatsächlich in der Titelzeile gezeichnete Dokumentüberschrift behält ihre
bisherige Überschriftenrolle im Dokument. Ein zusätzlicher dekorativer Kartentitel
steht als separate Beschriftung mit dem Namen „Dokumenttitel: …“ außerhalb des
Dokumenttexts. Dadurch werden Wörter und Schriftattribute des Dokuments nicht
dupliziert. Ohne Markdown-Überschrift bleibt der vorhandene Dateinamen-Rückfall
der Anwendung erhalten. Native Prüfungen adressieren das Dokument über seine
stabile Autor-ID, wenn der sichtbare Titel denselben Text enthält.

## Glyphenplan und Eigentum

Die Rechtecke stammen aus genau dem einfachen Glyphenplan, der den Textbefehl
zeichnet. Die native Aufbereitung rekonstruiert dessen unveränderliche Absatz-/
Fontzuordnung für GDEF-Cursorpositionen; die vorhandenen Glyphen werden nicht
erneut geformt. Native Läufe werden nur bei Bedarf erzeugt und im bestehenden
16-MiB-Plain-Cache mitgezählt. Bei fehlendem Platz verdrängt der Cache andere
Einträge; ein zu großer Export erhält eine kurzlebige vollständige Ausgabe.

Jeder Export besitzt seine Laufarrays selbst. Ein anderer Ursprung verschiebt
die kopierten Rechtecke, ohne Unicode/Wörter erneut zu segmentieren. Änderungen
oder Freigabe einer Ausgabe ändern den Cache nicht. Schriftneuladen und normale
Verdrängung entfernen die native Geometrie zusammen mit dem zugehörigen Plan.
Ein vollständig transparenter Textbefehl meldet keine gezeichneten Zeichen.

Die bestehenden umgebrochenen nativen Beschriftungen verwenden den bereits
gemeinsamen Wrap-Plan. Nummerierte Listen übernehmen ihre wirklichen Textbefehle.
Aufzählungspunkte übernehmen die Fläche des tatsächlich gezeichneten Kreises,
statt eine unbenutzte Schriftglyphe zu messen. Logische Tabellenköpfe ohne eigenes
Widget behalten ihre Semantik ohne erfundene sichtbare Glyphenfläche.

## Abnahme und verbleibende Arbeit

Der unabhängige Glyphen-/GDEF-Vergleich prüft Latein/Ligaturen/Akzente, Hebräisch,
Arabisch und Emoji bei100/150/200 Prozent Schrift, verschobene Ursprünge,
Ausgabeeigentum und transparente Befehle. Die produktive Prüfung erhält den
vollständigen Namen eines langen gekürzten Titels, prüft seine dargestellte Quelle,
nummerierte Marker, Kreisflächen, den Dateinamen-Rückfall und Dialogtitel.
Der echte Mac-Provider gleicht einen sichtbaren Titelbuchstaben über den
Dokument-Textbereich mit Breite, Höhe und Bildschirmposition ab.

Ausgeführte Gesamt-/Sanitizer-/Paketnachweise stehen in [STATUS](STATUS.md).
Tatsächliche VoiceOver/NVDA/Orca-, IME-, Geräte-/Monitor-/Langzeitabnahmen und die
neue Gesamtprüfung auf allen Zielplattformen bleiben Release-Arbeit.
