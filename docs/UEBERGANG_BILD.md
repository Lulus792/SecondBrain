# Ausgangsbild und Größenwechsel beim Dokumentübergang

Stand: 12. Oktober 2026. Umsetzung ab 0.9.68.

## Befund und Verhalten

Der frühere native Pixelvergleich las den Fensterpuffer nach `Present`. Dieser
Puffer ist bei Metal keine verlässliche Kopie des gerade gezeichneten Bilds.
Die Prüfung hält nun eine vorher gelesene RGBA-Kopie. Der ursprüngliche Vergleich
mit dem Ausgangsbild bleibt erhalten; die Übergangstextur besteht ihn tatsächlich.
Auch Testaufnahmen und Hinweisfarben verwenden dieselbe aktuelle Kopie.

Ein zusätzlicher realer Größenwechsel fand einen Produktfehler: Die gespeicherte
Karte blieb bei geändertem Fenster aktiv. Ihre Quellkoordinaten wurden mit den
neuen statt den aufgenommenen Fenstermaßen berechnet. Das konnte die alte Karte
falsch zuschneiden oder versetzen.

Die Aufnahme hält jetzt logische Fenstergröße und Pixelauflösung. Bei einer
Änderung wird die alte Textur freigegeben und ihre Überblendung beendet, bevor
die neue Oberfläche gezeichnet wird. Kamerafahrt, Auswahl und Dokument bleiben
erhalten. Ein neu aufbereitetes Dokument darf anschließend eine neue Aufnahme
mit passenden Maßen für seinen Ladeübergang anlegen. Normale Bilder werden
weiterhin direkt gerendert; es gibt keine zusätzliche CPU-Bildkopie in der App.
Die CPU-Kopie gehört ausschließlich zum Entwicklungsprüfer.

## Nachweise

Die erweiterte native Interaktionsprüfung besteht mit Metal auf Intel/macOS
14.6.1/23G93, Retina-Dichte 2.00: 173 Bedingungen. Der geformte Anfangsreader
wird begrenzt bis zur tatsächlichen Bereitschaft abgewartet. Die erste Aufnahme
ist pixelgleich zum Ausgangsbild; zwölf schnelle Wechsel und die Aufnahme nach
Resize stimmen mit dem im jeweiligen Frame gezeichneten Fensterbild überein.
Aufnahmegrößen stimmen mit den tatsächlichen Bildpixeln überein. Verkleinern auf
1000×700 und Rückkehr auf 1336×840 beenden die alte Überblendung ohne Verlust
von Notiz oder Kamerafahrt. Der Resize-Fall ist vor der Korrektur tatsächlich
negativ ausgeführt. Bestehende Scroll-, Hinweis-, Dialog-, Fokus- und Aktions-
bedingungen wurden erhalten.

Sieben lokale Release-Prüfungen für Material, Interaktion, Vordergrundebenen,
Modalfokus, Navigation/Suche/Textcache und beide Versionswege bestehen (104,35 s).
Die Software-Interaktionsprüfung enthält ebenfalls 173 Bedingungen.
Ausgangs-, Zwischen- und Zielaufnahmen aus Metal wurden angesehen.
173 Bedingungen bestehen zusätzlich unter ASan/UBSan; 36 eigene C-Dateien
instrumentiert und 58 aktuelle Quell-/Includehashes bestätigt. Externe UI nicht
instrumentiert, Leakerkennung aus, kein ThreadSanitizer. Kein neuer vollständiger
lokaler 84er-Lauf. Paket und Installation sind nachfolgend dokumentiert.
Originale: `build/transition-pixel-verification/native.log`, `resize-negative.log`,
`native-fixed.log` und die Interaktionsbilder. Diagnosezwischenstände sind keine
Abnahmen; der Prüfer erlaubt ausdrücklich eine neue passende Ladeaufnahme nach
Reflow, statt eine legitime neue Textur als Fehler zu behandeln.

## Grenzen

Dies belegt die genannten GPU-/Fenster-/Texturzustände, keine menschliche
Flüssigkeits-, Trackpad- oder Display-Scanout-Abnahme. Die erste Pixelaufnahme
verwendet 100 % Schrift und die genannten Größen; die weiteren Dialogbedingungen
umfassen 100/200 %. Ein echter Wechsel zwischen Monitoren mit anderer Pixeldichte,
weitere Geräte sowie Windows/Linux/ARM64 bleiben getrennt zu prüfen. 1.0 wird
weiterhin erst nach ausdrücklicher Nutzerfreigabe gesetzt.

## Sauberes Paket und Installation

Intel/macOS-Release 0.9.68/0b0fdacda4fa; Versionsprüfungen bestehen (0,48 s).
Entpacktes Paket: 126 Desktop-, 167 Tastatur-, 75 Sicherungsbedingungen, zwei
Einstellungsprozesse, CLI-/Runtime-/Lizenz-/Versionsprüfung und Wiederöffnung mit
20 unveränderten Dateien. Echter 67→68→67-Wechsel erhält Dateien/Einstellungen;
Sicherung/Wiederherstellung bestehen, drei App-Aufnahmen angesehen. 98 Dateien
paketgleich installiert; 18 eigene Gedächtnisdateien beim Öffnen unverändert,
installierte Aufnahme angesehen. Binärminimum macOS 14.0/SDK 15.2; tatsächlich
ausgeführt auf Intel/macOS 14.6.1/23G93. Minimum/ARM64 nicht damit abgenommen.

Archiv-SHA256: `5ed3f957a77568cf899bcce2b139214ca236dbf575424664490420139166b136`.
Rückfallkopie: `/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.67-20261012-005030`.
Originale: `build/transition-pixel-package-check`, `build/transition-pixel-update`,
`build/transition-pixel-verification/installed.json` und `installed-brain-check.json`.
CI38181757771 zu 0b0fdac: vier Windows-/Linux-Desktopjobs in Arbeit, Mac queued.
