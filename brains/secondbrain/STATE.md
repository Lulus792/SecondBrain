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

Installiert **0.9.58 / 4d984a6330c2**, sauberer Intel/macOS-Release-Build.
96 Dateien entsprechen dem geprüften Paket; 18 Gedächtnisdateien beim Öffnen
unverändert. 126 Desktop-, 161 Tastatur-, 75 Sicherungs-Assertions, zwei Neustarts,
produktive CLI und Runtime-/Lizenz-/Version bestehen. Tatsächliche App-Aufnahme
angesehen. Prüfgerät: macOS 14.6.1/23G93, x86_64.
Rückfallversion 0.9.57: `/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.57-20261011-191238`.
CI 38158192485 zum Quellcommit zuletzt queued; neue Gesamt-Plattformabnahme offen.
Frühere Plattformnachweise ersetzen keine Prüfung dieser neuen Fassung.

## Geprüfte aktuelle Änderung

0.9.58 erhält Reader-/Quellpläne beim Öffnen/Schließen gewöhnlicher Menüs,
Reader-/Editorposition und Inhaltshöhe. Die native Modalabschirmung bleibt.
115 Reader-, 177 Quell-/Stil- und 621 Mac-Assertions auch unter ASan/UBSan;
39 eigene C-Dateien, 55 aktuelle Hashes. 78/78 Release-Prüfungen bestehen
(651,31 s). Finale Vorher/Menü/Nachher-Bilder angesehen; SBUI-079 lokal geprüft.
SBUI-080 bleibt offen: F1 verliert die Ursprungsaufgabe eines begonnenen Formulars.
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

SBUI-080 korrigieren: Hilfe/vergleichbare Befehlswechsel aus begonnenen Formularen
erhalten Ursprungsaufgabe, Eingaben, Fokus/Auswahl und Scrollposition.

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
