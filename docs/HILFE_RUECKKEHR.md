# Vom Hilfefenster zurück zur Aufgabe

Stand: 11. Oktober 2026. Umsetzung ab 0.9.59, Befund SBUI-080.

## Verhalten

F1 öffnet die Tastaturhilfe vorübergehend. War bereits ein Formular oder Menü
geöffnet, führen Escape und der Zurück-Pfeil wieder dorthin. Formularwerte,
Wissensbereich, manuell gewählter Dateiname, Fehlermeldung, Fokus, gerichtete
Textauswahl und Scrollposition bleiben erhalten. Wiederholtes F1 öffnet keine
weiteren Ebenen. Aus der Hauptansicht geöffnete Hilfe schließt wie bisher.

Umschalt+F10 öffnet Dokumentaktionen im Arbeitsbereich. Ein begonnenes Formular,
die Hilfe oder eine andere modale Aufgabe wird davon nicht ersetzt. Während
einer Sicherung oder einer Entscheidung über ungespeicherte Änderungen ersetzt
F1 die laufende Aufgabe nicht.

## Umsetzung und Besitz

Die Hilfe hält einen einzelnen Rückkehrzustand. Die Formularwerte gehören weiter
dem Desktop; Auswahlkopien wechseln ihren Besitzer und werden bei Rückkehr oder
Freigabe genau einmal freigegeben. Rückkehr setzt die Auswahl zur Wiederaufnahme
im tatsächlich gezeichneten Textfeld. Alte native Eingabeaktionen bleiben durch
die vorhandenen Formular-/Veröffentlichungskontexte ausgeschlossen.

Der Dialogkörper erhält seinen gehaltenen Scrollwert auch im Nuklear-Fenster.
Ein schon beendeter Mauskontakt startet nach Rückkehr kein altes Scrollbar-Ziehen.
Ein echter Projekt-/Dokumentgenerationswechsel verwirft die vorherige Aufgabe.
Die Hilfe führt keine solchen Wechsel aus. Vorläufige IME-Komposition wird beim
Hilfeeinstieg wie bei anderen Fokusgrenzen abgebrochen; bestätigte Eingaben bleiben.

## Prüfungen und Grenzen

Der neue produktive Prüfer [test_help_return.c](../tests/test_help_return.c)
bestätigt vor der Korrektur, dass Umschalt+F10 ein begonnenes Notizformular ersetzt.
Die frühere F1-Probe ist in [Reader-Rückkehr](READER_RUECKKEHR.md) dokumentiert.

92 Prüfbedingungen bestehen lokal: Notiz, Projekt, Arbeitsordner, Sicherung und
Wiederherstellung; F1-Wiederholung; Escape und Headerknopf; erhaltene Fehlermeldung,
Werte und Auswahl; Unicode-Grapheme und umgekehrte Auswahl; verworfene alte native
Aktion und anschließende Ersetzung; 780×520/150%-Schrift mit gescrollten Settings;
modaler Kontextreader; Hilfe aus dem Aktionsmenü über Taste und Schaltfläche;
eigenständige Hilfe, Schutzdialog und Freigabe während einer Hilfeaufgabe.

Vorher/Hilfe/Nachher-Aufnahmen der tatsächlichen Softwareausgabe wurden betrachtet;
die zurückgekehrte Notiz zeigt weiterhin Titel, Dateiname, Bereich, Fehlermeldung
und markierten Titelteil. Die Hilfe zeigt bei vorhandener Ursprungsaufgabe einen
Zurück-Pfeil. Originale: `build/help-return-baseline.log`,
`build/help-return-test.log`, `build/current-ui/help-return-46079/`.

92 Hilfe-, 190 Feldaktions- und 621 native Mac-Prüfbedingungen bestehen auch
unter haltendem ASan/UBSan. 39 eigene C-Dateien und 56 Quell-/Include-Hashes
wurden erfasst; die Quellen waren vor und nach dem Lauf identisch.
Originale: `build/help-return-sanitizers/`. Externe UI-Bibliotheken wurden nicht
instrumentiert; LeakSanitizer ist auf diesem Mac deaktiviert, ThreadSanitizer
wurde nicht verwendet.

Alle 79 lokalen Release-Prüfungen bestehen (565,40 s). Die vollständige Prüfung
umfasst auch den bisherigen Desktop-/Tastatur-/Sicherungsablauf, Reader,
Texteingabe, native Mac-Anbindung, Material und Versions-/Release-Verträge.
Original: `build/help-return-verification/ctest-release.log`.

Die erste Paketprüfung bestand den Desktopablauf, scheiterte aber im bestehenden
Tastaturprüfer: Bild ab wurde vor abgeschlossener Vorbereitung eines langen
Lizenztexts ausgelöst. Der Prüfer verwendet jetzt seine vorhandene begrenzte
Bereitschaftsprüfung vor dem Scrolltest. Der gezielte Tastaturdurchlauf danach
besteht (53,12 s); die Produktlogik blieb dabei unverändert. Frühe Scrollbefehle
während der Vorbereitung bleiben eine eigene offene Leistungs-/Bedienaufgabe.
Originale: `build/help-return-verification/package-check.log`, `keyboard-ready.log`.

Das sind SDL-Ereignis-, Widget-, Provider- und Bildnachweise auf Intel/macOS,
keine menschliche Tastatur-/Screenreader-/IME-Abnahme oder aktuelle Windows-/
Linux-/ARM64-Abnahme. Leistung und übrige [Release-Aufgaben](RELEASE.md) bleiben
offen. Die Versionsnummer 1.0 benötigt weiterhin ausdrückliche Nutzerfreigabe.
