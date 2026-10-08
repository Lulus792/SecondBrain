# Schriftbesitz für die Textvorbereitung

Stand: 8. Oktober 2026, Unterbau ab 0.9.44, formatierter Umbruch ab 0.9.45.
Der Prototyp wird noch nicht vom Dokumentwechsel aufgerufen. Er bereitet nun
auch formatierte Texte mit Absätzen und Umbruch vor; vollständige
Dokumentstruktur und sichtbare Ladezustände bleiben weitere Integration.
[Gesamtvertrag](READER_VORBEREITUNG.md).

## Unveränderliche Ressourcen

Die UI liest ihre neun Schriftdateien in einen unveränderlichen Ressourcensatz
mit insgesamt höchstens 64 MiB. Alle eigenen Rollen und Stilvarianten verwenden
diese Daten. Jedes Öffnen besitzt einen eigenen SDL-IOStream; veränderliche
Dateipositionen werden nicht mit Worker-Fonts geteilt. Referenzzählung hält die
Bytes auch nach Schriftneuladen bis zum letzten Besitzer gültig.

Ein auf dem UI-Thread erfasster Snapshot enthält Ressourcenidentität, Schrift-
Epoche, Rollen-/Stilkennung, Größe, Hinting, Outline, Kerning, SDF, Sprache und
Generation aller acht beteiligten Fonts. Schriftneuladen ersetzt den Satz und
die Epoche. Eine bloß gleiche Dateiadresse ist kein Nachweis gleicher Fontdaten.

## Private Worker-Fonts

Jede Instanz öffnet eigene Fonts aus den Snapshotdaten und verwendet/schließt
sie auf ihrem Erzeugerthread. Die Instanz hält ihre eigenen Ressourcenreferenzen
und eine SDL_ttf-Initialisierungsreferenz. Der Snapshot darf deshalb schon
freigegeben sein, während eine Instanz noch mit ihren Fontdaten arbeitet.
UI-Fonts, Renderer und Texturen werden nicht auf den Worker übertragen.

Die Fontauswahl folgt derselben Graphem-/Emoji-/Neutralzeichenregel wie die UI.
Sprache und Stilparameter werden übernommen. Glyphenpläne transportieren nach
der Arbeit nur Zahlen und Schriftkennungen; alle Worker-Fontzeiger werden vor
dem Schließen entfernt. UI-Bindung prüft Ressourcensatz, Epoche, Generation und
Erzeugerthread, bevor sie wieder lebende UI-Fonts einsetzt.

Primärvertrag: festgelegtes SDL_ttf3.2.2-Headerkapitel zu TTF_OpenFontIO,
TTF_CopyFont, Hinting/Outline/Kerning/SDF/Language. Vorhandene UI-Fonts werden
nicht einfach von einem anderen Thread benutzt. Der eigene Sprachgetter im
SDL_ttf-Hook liest auf dem Font-Erzeugerthread, ohne Einstellungen zu ändern.

## Vorbereitungsjob

`app/prepare.c` kopiert Text und Dokumentkontext vor dem Threadstart. UTF-8-
Validierung, Script-/Bidi-Auswertung, Fontwahl und Formung laufen auf dem Worker.
Ein atomarer Abschluss veröffentlicht das vollständige Ergebnis. Zustände:
ausstehend, bereit, fehlgeschlagen, abgebrochen. Abbruch selbst wartet nicht.

Die Übernahme prüft exakte Quellenbytes, Länge, Kontext und aktuelle Schrift-
bindung. Sie verändert keine Datei und transferiert die Geometrie nur einmal
in eine zunächst leere, anschließend vom Empfänger besessene Ausgabe. Bereits
abgebrochene oder veraltete Ergebnisse werden abgewiesen. `sb_prepare_free`
wartet gegebenenfalls auf den Thread; spätere Navigation muss abgebrochene
Jobs deshalb zunächst zur nichtblockierenden Abschlussprüfung zurückstellen.

## Tatsächlicher Prüfweg und Grenzen

Ab 0.9.45 erfasst ein Textsnapshot alle bereits vorhandenen Rollen/Stilfonts,
ohne zuvor unbenutzte UI-Varianten zu erzeugen. Ein CPU-Kontext öffnet private
Fonts auf seinem Thread, besitzt keine Fenster/Renderer und benutzt denselben
Layoutalgorithmus wie die Leseansicht. Neue Stilvarianten entstehen dort aus
den eigenen Basisfonts. Bei der UI-Bindung müssen nachträglich entstandene
Stilfonts den erwarteten Parametern entsprechen; fremd veränderte Varianten
werden nicht ungeprüft eingesetzt.

Ein Layoutjob besitzt Quellenbytes und Stilspannen sowie Rolle, Breite,
Innenabstände, Zeilenabstand, Abschlusszeilenregel und Dokumentkontext. Er liefert
Zeilen, Quellenoffsets, Positionen und Gesamthöhe. Seine Fontkennungen werden
einmalig an aktuelle UI-Fonts gebunden. Die Freigabe seiner Kennungsarrays bleibt
auch nach Übertragen des Plans unabhängig vom Planbesitz. Abbruch wird zwischen
Absätzen, in der Fontwahl und zwischen Umbruchzeilen berücksichtigt; einzelne
Bibliotheksaufrufe können dennoch erst nach ihrer Rückkehr beendet werden.

Der neue Prüfer vergleicht die vollständigen Glyphen-/Lauf-/Font-/Metrikwerte
mit dem normalen UI-Plan bei 100/150/200 Prozent Schrift, Latein/Ligaturen/
Akzenten, Hebräisch, Arabisch, Emoji und allen Stilvarianten. Spracheinstellung,
leerer Text, unabhängige Eingabekopie, doppelte Übernahme, veränderter Kontext,
Abbruch, ungültiges UTF-8, Snapshotfreigabe und Schriftneuladen sind erfasst.
Fremdthread-Erfassung/Bindung wird abgewiesen.

Dieser Unterbau ist kein Nachweis eines flüssigen langen Dokumentwechsels.
Es fehlen insbesondere eine begrenzte Jobverwaltung, Dokumentblöcke/Tabellen
mit ihren jeweiligen Breiten, Output-/Spitzen-Speicherabnahme großer Texte, nicht-
blockierende Erholung/Shutdown und Integration in den sichtbaren Framepfad.
Der Job akzeptiert höchstens die vorhandene Textgrenze; die Fontdaten-Grenze
ist kein allgemeines 64-MiB-Limit aller Bibliotheks-/Glyphenallokationen.
Ausgeführte Tests, Sanitizer-Scope und neue Plattformnachweise stehen in
[STATUS](STATUS.md). Der vollständige Auftrag und die Sperre für 1.0 bleiben.
