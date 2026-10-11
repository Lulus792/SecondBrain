# Reader-Aktionen bei knapper Breite

Stand:11.Oktober2026, Entwicklungsschritt0.9.65. Der Reader verteilte seine
Aktionen bisher auf gleich breite Spalten. Bei großer Schrift und schmaler Karte
entstanden mehrere Wortreste, obwohl Icons und vollständige Zielnamen vorhanden
waren. Der bisherige Renderer bestätigt die neue negative Regression mit
„Bearb…“ statt „Bearbeiten“.

Die eigene C-Aktionsreihe misst nun die tatsächlich verwendete Schrift und
berücksichtigt Icon-, Text- und Spaltenabstände. Wenn alle Beschriftungen passen,
erhalten sie ihre benötigte Breite. Sonst priorisiert die Reihe Bearbeiten,
beziehungsweise Speichern im Editor, und verwendet bei anderen Aktionen die
vorhandenen Icons. Eine Aktionsbeschriftung ist entweder vollständig sichtbar
oder als Icon mit vollständigem Hover-/Zielnamen erreichbar. Die normale
Kürzung langer Dokumenttitel und verlinkter Inhalte bleibt davon unabhängig.

Der Quellenreader priorisiert „Zum Dokument“. Gespeicherter Reader, Entwurf,
Editor und Quelle behalten dieselben IDs, Reihenfolge und Befehle. Es entsteht
kein zusätzliches Menü und keine weitere Bestätigung. Tastaturaktivierung und
native Namen verwenden weiter die vorhandenen Ziele. Die Glasdarstellung und
die C-Implementierung der Anwendung bleiben erhalten.

Grundlage: Apple [Toolbars](https://developer.apple.com/design/human-interface-guidelines/toolbars)
und [Buttons](https://developer.apple.com/design/human-interface-guidelines/buttons),
am11.Oktober erneut über titelgeprüfte offizielle DocC-Daten gelesen.
Die konkrete Breitenverteilung ist eine eigene Übertragung auf das vorhandene
Toolkit. Quellaufnahmen, Regelbezug und stabile ID SBUI-084 stehen im
[UI-Bericht](/Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-11_22-45-48/UI_REVIEW.md).

## Tatsächliche Prüfung

Der [Produktprüfer](../tests/test_reader_actions.c) besteht lokal mit800
Caption-/Geometrie-/Aktivierungsbedingungen über36 Kombinationen aus
100/150/200Prozent Schrift,780/1000/1336 logischer Fensterbreite und vier
Reader-/Editor-/Quellenzuständen (11,95s). Er untersucht die tatsächlichen
Textkommandos innerhalb jedes Buttons, nicht nur dessen vollständigen nativen
Namen. Aktionen/Escape, Vergrößern und Bearbeiten bleiben aktivierbar.
Betrachtete neue Reader-/Editor-Aufnahmen:150Prozent,1000×840. Die negative
Aufnahme zeigt einen ungespeicherten Reader bei100Prozent/1000×840; sie ist kein
pixelweises A/B-Paar zu den betrachteten neuen Bildern. Die Matrix prüft denselben
negativen Zustand zusätzlich mit dem neuen Renderer.

Separater Intel/macOS-Release-Build unter `build/reader-actions-review/`.
Zehn weitere passende Produktprüfungen bestehen (185,15s), einschließlich
nativer Zugänglichkeit, Bewegung/Layout, Ebenen, modalem Fokus, Beschriftungen,
Desktop-/Tastaturabläufen, zwei Versions- und Releaseprüfungen. Kein neuer
83er-Gesamtlauf behauptet. Sauberes Paket und Installation folgen vor dessen
Freigabe. Native Windows-/Linux-/ARM64-, weitere Darstellungs- und menschliche
assistive Abnahmen bleiben offen. SBUI-074 und die übrige [Release-Liste](RELEASE.md)
werden durch diesen Aktionsschritt nicht abgeschlossen.

## Geprüftes Paket installiert

Sauberer Intel/macOS-Release0.9.65/1a19bee557ca: drei abschließende Reader-/
Versionsprüfungen bestehen (8,32s). Entpacktes Paket besteht126 Desktop-,167
Tastatur-,75 Sicherungsbedingungen,20-Datei-Wiederöffnung, zwei Einstellungs-
prozesse und CLI-/Runtime-/Versionsprüfung. Echter Wechsel64→65→64 erhält die
Testdateien, gewählte Notiz und Darstellung; Sicherung/Wiederherstellung bestehen.
98 installierte Dateien paketgleich,18 echte Projektgedächtnisdateien beim
Öffnen unverändert. Installierte App-Aufnahme angesehen. Archiv-SHA256:
`b40958a6e001a8525efe85e6fdc6f440a14116c44f31eaa8cad2f44658d2bc74`.
Rückfall: `build/previous-dist-0.9.64-20261011-230732`.
Originale: `build/reader-actions-verification/`, `build/reader-actions-package-check/`.

CI38174603923 zu1a19bee: Linux-Release einschließlich Paket erfolgreich;
Windows/Linux-Debug noch in Arbeit, Mac-Jobs queued. Das ist keine menschliche
Screenreader-/Geräte-/allgemeine Stilabnahme.47 ausführbare Python-Prüfungen
bestehen, eine native Windows-Prüfung lokal übersprungen.
