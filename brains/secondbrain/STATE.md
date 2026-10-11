# SecondBrain: aktueller Stand

Stand: 11. Oktober 2026. Originale: [STATUS](../../docs/STATUS.md),
[Release-Liste](../../docs/RELEASE.md), [Quellen](SOURCES.md).
Historische Schritte stehen im [Journal](journal/2026-10-11.md) und in STATUS.

## Produkt und installierte App

Eigene C17-App für Mensch und KI: Projekte/Notizen anlegen, lesen, bearbeiten,
suchen, archivieren, sichern; Quellen lesen und gespeicherten Kontext kopieren.
Lumen-Sternkarte, Glaskarten, eigene Icons, direkte Pfeilnavigation, Kamerafahrt,
Scrollen, Startfokus, Textcursor und große Leseansicht sind implementiert.
Originalbytes, Entwürfe und erkannte Konflikte bleiben geschützt. Externe
Bibliotheken bleiben auf UI beschränkt; eigene Lizenz MIT.

Installiert **0.9.59 / 4f4d8fb9da88**, sauberer Intel/macOS-Release-Build.
96 Dateien entsprechen dem geprüften Paket; 18 Gedächtnisdateien beim Öffnen
unverändert. 126 Desktop-, 166 Tastatur-, 75 Sicherungs-Assertions, zwei Neustarts,
produktive CLI und Runtime-/Lizenz-/Version bestehen. Tatsächliche App-Aufnahme
angesehen. Prüfgerät: macOS 14.6.1/23G93, x86_64.
Rückfallversion 0.9.58: `/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.58-20261011-195420`.
CI 38161180159 zum aktuellen Quellcommit zuletzt queued. Der frühere Lauf
38158192485 bestätigt Windows-UI-Buildfehler und zwei Linux-UI-Fehler.
Neue Gesamt-Plattformabnahme offen.
Frühere Plattformnachweise ersetzen keine Prüfung dieser neuen Fassung.

## Geprüfte aktuelle Änderung

0.9.58 erhält Reader-/Quellpläne beim Öffnen/Schließen gewöhnlicher Menüs,
Reader-/Editorposition und Inhaltshöhe. Die native Modalabschirmung bleibt.
115 Reader-, 177 Quell-/Stil- und 621 Mac-Assertions auch unter ASan/UBSan;
39 eigene C-Dateien, 55 aktuelle Hashes. 78/78 Release-Prüfungen bestehen
(651,31 s). Finale Vorher/Menü/Nachher-Bilder angesehen; SBUI-079 lokal geprüft.
SBUI-080 ist ab 0.9.59 lokal korrigiert: Hilfe kehrt zur Ursprungsaufgabe zurück.
92 Hilfe-, 190 Feldaktions- und 621 native Mac-Prüfbedingungen auch unter
ASan/UBSan; 39 eigene C-Dateien/56 Hashes. 79/79 lokale Release-Prüfungen
bestehen (565,40 s); danach geänderter Lizenz-Tastaturprüfer separat bestanden.
[Hilfe-Vertrag](../../docs/HILFE_RUECKKEHR.md). Finales Paket/Installation geprüft.
[Vertrag](../../docs/READER_RUECKKEHR.md). Intel/macOS-Paket/Installation geprüft;
weitere aktuelle Zielplattformabnahmen offen.

Zusätzlich bleiben graphemgebundene Stilgrenzen im Quellenbudget gehalten;
Schrift- und Zeichenpositionen bleiben aktuell. [Stilplan](../../docs/READER_STILPLAN.md).

0.9.56 hält Inline-Analysen von Absätzen/Überschriften mit Text und Referenzen
im vorhandenen 16-MiB-Quellbudget. Iteratoren erhalten eigene Cursor; Änderungen
ersetzen den gesamten Plan. Speicher-/Budgetrückfälle bleiben vollständig.
Vor einem fehlgeschlagenen Quellenersatz werden frühere geliehene Ausgaben
zurückgesetzt. [Vertrag](../../docs/READER_INLINE_PLAN.md).

## Weitere implementierte Grundlagen

Der Reader hält seinen Dokumentbaum und genaue Glyphen-/Umbruchpläne. Lange
formatierte Dokumente erhalten einen privaten Vorbereitungs-Worker; schnelles
Retargeting wartet nicht auf dessen Abschluss. Native Editor-/Formular-/Reader-
und Titelgeometrie folgt den gezeichneten Plänen. Die native Mac-Auswahl und
Ersetzung sind an die vorhandenen Feldaktionen gebunden. Umfang und Grenzen:
[Reader](../../docs/READER_VORBEREITUNG.md),
[Textgeometrie](../../docs/NATIVE_TEXTGEOMETRIE.md),
[Leseblöcke](../../docs/NATIVE_LESEGEOMETRIE.md).

Dialogglas verdeckt konkurrierende Hintergrundtexte und erhält das Material;
Icons und Exportzuordnungen sind nachgeprüft. [Dialogmaterial](../../docs/DIALOGMATERIAL.md).
Die [Frame-Messung](../../docs/FRAME_MESSUNG.md) erfasst Eingabe, erste Bilder und
reale Animationsabstände; Einzelausgaben ersetzen keine menschliche Bedienabnahme.

## Nächste Arbeiten und verbleibende Abnahmen

0.9.60 korrigiert Cache-Indexierung an `void *`; GCC/Clang weisen sie künftig
als Fehler ab. Fensterprüfer synchronisieren und bestätigen 780×900 bzw.
780×520; der AT-SPI-Prüfer sucht den Inhalt im passenden Textbereich.
525 Cache-, 117 Reader-, 93 Hilfe- und 621 Mac-Prüfbedingungen bestehen auch
unter ASan/UBSan: 40 eigene C-Dateien/57 Hashes. Lokale gezielte Nachprüfung
besteht. Gesamt- und neue native Zielplattformnachweise folgen; installiert
bleibt 0.9.59. [Vertrag](../../docs/UI_PLATTFORMKORREKTUREN.md).

Als nächstes die bestätigten Plattformbefunde korrigieren: Windows C2036 in
`plain_native.inc:35`; Linux-Reader nach Resize und native Textstilabfrage
„Stilprüfung“. [Originalbefunde](../../docs/RELEASE.md). Die frühere bloße
Queued-Beobachtung des 0.9.58-Laufs ist dadurch ergänzt, kein Gesamtpass.
Die erste 0.9.59-Paketprüfung fand einen frühen Scrollbefehl während der
Lizenzreader-Vorbereitung. Der Tastaturprüfer wartet für seine fertige Ansicht
jetzt begrenzt; sein gezielter Durchlauf besteht. Frühe Scrollbefehle bleiben
als Produktarbeit offen. Das finale Paket mit dem neuen Prüfer besteht.

Native Blockausgaben/-veröffentlichung, erste Quell-/Parser-/Worker-Aufbereitung,
Tabellenzellen, Rasterung und Fälle über den Vorbereitungsbudgets getrennt
profilieren. Synchrone Rückfälle, große Einzelabsätze und Shutdown bleiben
Leistungsfälle. SBUI-074 ist für die vollständige Geräte-/Großdatenabnahme offen.

Aktuelle Windows-/Linux-/ARM64-Prüfungen, tatsächliche IME und VoiceOver/NVDA/Orca,
native Dialog-/Geräte-/Monitor-/Langzeitprüfungen, Windows-/Linux-Speicherfehler,
Mindest-OS und frische Rechner fehlen. Native Snapshot-Pixelidentität und
Speicherfehlerabnahme der Material-Ersatzfläche bleiben getrennt offen.
Vollständige Runtime-/Lizenzprüfung, vertraulicher Sicherheitskanal, Signierung,
dauerhafte aktuelle Release-Pakete und abschließende natürliche Texte bleiben
Release-Arbeit. Apple-Developer-Konto und Windows-Signaturzertifikat fehlen.
**1.0 nur nach ausdrücklicher Nutzerfreigabe.**
