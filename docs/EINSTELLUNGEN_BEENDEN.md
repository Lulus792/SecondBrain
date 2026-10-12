# Einstellungsfehler beim Beenden

Stand: 12. Oktober 2026. Umsetzung ab 0.9.72.

Vorher speicherte main.c die Einstellungen erst nach Ende der Ereignisschleife.
Ein Fehler wurde nur nach stderr geschrieben, danach wurde das Fenster zerstört.
GUI-Nutzer erhielten weder eine sichtbare Rückmeldung noch eine Wiederholung.

Normaler Quit prüft die Speicherung jetzt vor dem Fensterabbau. Bei Fehler bleibt
die App offen und zeigt eine eigene Glaskarte mit Fehlergrund und drei Aktionen:
Erneut versuchen, Ohne Einstellungen beenden, Abbrechen. Die erste Aktion ist
standardmäßig fokussiert. Escape und Command-Punkt brechen ab; Tab, Pfeile,
Return/Leertaste und Maus bedienen dieselben Ziele. Wiederholtes Fensterschließen
oder andere Shortcuts umgehen die Entscheidung nicht.

Abbrechen erhält die vorherige Ansicht, Formulareingaben, Fokus und Scrollzustand.
Ein bewusstes Überspringen schreibt nicht erneut. Bereits gespeicherte Dokumente
werden dadurch nicht verworfen; der bestehende Notizschutz bleibt vorgeschaltet.
Die normale Speicherung erfolgt einmal. Snapshot-CLI-Prozesse zeigen keinen
versteckten interaktiven Dialog und melden einen abschließenden Fehler mit Exit 1.

Die Karte verwendet die vorhandene Guard-Geometrie mit festen Aktionen und
scrollbarem Fehlertext sowie eigene regenerativ zugeordnete Icons. Grundlage:
[Apple Alerts](https://developer.apple.com/design/human-interface-guidelines/alerts),
am 12. Oktober über offizielle DocC-Daten gelesen. Konkreter Titel, handlungsfähige
Aktionen und Abbruchwege sind eigene Übertragung; Pixelmaße bleiben Projektentscheidungen.

Lokal bestehen 83 neue Bedingungen über drei Schriftgrößen und 780×520; die
200-%-Karte wurde angesehen. Alte Einstellungen bleiben bei Fehler/Überspringen
bytegleich, Wiederholung gelingt und ein begonnener Notizdialog kehrt mit Text
und Fokus zurück. Die erste Prüfung hatte einen unsynchronisierten Editor im
Testaufbau; das wurde korrigiert. Vier bestehende Guard-/Help-/Preferences-Prüfungen
bestehen, sieben abschließende Gruppen einschließlich nativer Mac-Anbindung und
Modalfokus bestehen in 55,29 s. Originale: `build/exit-preferences-verification/`.

Diese ersten gezielten Prüfungen wurden durch die abschließenden Nachweise
unten ergänzt. Keine menschliche Desktop-/Screenreaderabnahme.


Die abschließende Erweiterung prüft auch eine bereits geöffnete Hilfe und eine
wirklich gescrollte Einstellungsansicht. Rückkehr bewahrt die Position; gehaltene
Scrollbar-Eingaben werden nicht wieder aktiviert, automatische Fokusoffenlegung
setzt die Position nicht zurück. Beide betroffenen Gruppen bestehen (16,04 s).
93 neue Fehler-/Rückkehrbedingungen und 94 Hilfe-/Auswahlbedingungen bestehen
unter ASan/UBSan mit 61 aktuellen Quell-/Includehashes. Eigene C-Teile sind
instrumentiert, externe UI nicht; Leaks aus, kein TSan. Originale:
`asan-final-exit.log`, `asan-final-help.log`, `scroll-regression.log`
und `build/exit-preferences-sanitizers/sources.json`. Iconexport reproduzierbar.


## Abschließende Gesamt-, Paket- und Installationsprüfung

Installiert: **0.9.72 / c40c9fd47ce1**, sauberer Intel/macOS-Release auf
macOS 14.6.1 / 23G93. 98 Dateien paketgleich. App-SHA256:
`3406d07cc09f239b5208ebaf50be6b76f7f692540b15826cc7c411acbe45c8a8`.
Alle **88 lokalen Release-Tests** bestehen (471,00 s); alle 209 gehashten
Produktinputs und die ausführbare Datei bleiben über den Lauf unverändert.
Das entpackte Paket besteht 126 Desktop-, 167 Tastatur- und 75 Sicherungsbedingungen,
Einstellungen in zwei Prozessen, CLI, Lizenzen, Versions- und Laufzeitprüfung.
Der echte **71→72→71-Wechsel** erhält 20 Testdateien und Einstellungen;
Sicherung/Wiederherstellung bestehen. Drei Updateaufnahmen und die installierte
Ansicht wurden betrachtet. Beim Öffnen bleiben 21 Gedächtnisdateien unverändert.
Deployment-Ziel der App: minos 14.0, SDK 15.2; kein Nachweis auf macOS 14.0/ARM64.
[Fehlerkarte und genaue Abdeckung](EINSTELLUNGEN_BEENDEN.md).
Originale unter `build/exit-preferences-verification/` und
`build/exit-preferences-update/`.

[CI38191364672](https://github.com/Lulus792/SecondBrain/actions/runs/38191364672)
zu c40c9fd: Windows und Linux jeweils **88/88 Tests in Debug und Release**;
beide Release-Pakete erfolgreich entpackt und geprüft. macOS-Jobs warten auf
Runner. Windows Server 2025 und Ubuntu/Xvfb ersetzen keine Windows-11-/Wayland-
oder menschliche Screenreaderabnahme.

Die bisherige Installation liegt als Rückfallkopie unter
`/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.71-20261012-033219`. ASan/UBSan und automatisierte SDL-Eingaben
ersetzen keine menschliche Bedienabnahme. 1.0 bleibt ohne Nutzerfreigabe offen.
