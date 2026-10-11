# SecondBrain: aktueller Stand

Stand: 11. Oktober 2026. Originale: [STATUS](../../docs/STATUS.md),
[Release-Liste](../../docs/RELEASE.md), [Quellen](SOURCES.md).
Historische Schritte stehen im [Journal](journal/2026-10-11.md) und in STATUS.

## Produkt und installierte App

Eigene C17-App für Mensch und KI: Projekte/Notizen anlegen, lesen, bearbeiten,
suchen, archivieren, sichern; Quellen lesen und gespeicherten Kontext kopieren.
Lumen-Sternkarte, Glaskarten, eigene Icons, direkte Pfeilnavigation, Kamerafahrt,
Startfokus, Textcursor und große Leseansicht sind implementiert. Originalbytes,
Entwürfe und erkannte Konflikte bleiben geschützt. Externe Bibliotheken nur UI; MIT.

Installiert **0.9.61 / e219bf47cbdb**, sauberer Intel/macOS-Release-Build.
96 Dateien entsprechen dem geprüften Paket; 18 Gedächtnisdateien beim Öffnen
unverändert. 126 Desktop-, 167 Tastatur-, 75 Sicherungs-Prüfbedingungen, zwei
Neustarts, CLI-/Runtime-/Lizenz-/Version bestehen. Tatsächliche App-Aufnahme
betrachtet. Gerät: macOS 14.6.1/23G93, x86_64.
Rückfall 0.9.60: `/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.60-20261011-210850`.

CI 38165889862 zu dc5af86/0.9.61: alle vier Windows-/Linux-UI-Jobs erfolgreich,
Debug/Release und beide Release-Pakete tatsächlich geprüft. Native Mac-Jobs
zuletzt queued; lokaler Intel-Nachweis separat. ARM64 und frische Rechner offen.
Frühere Nachweise ersetzen keine Prüfung neuer Änderungen.

## Geprüfte aktuelle Änderungen

0.9.61 erhält frühe Scrollbefehle während der Reader-Vorbereitung. Die erste
Inhaltshöhe wird vor der Anwendung gemessen; große Eingaben bleiben begrenzt,
Gegenrichtung und normale Animation erhalten. Notizwechsel verwerfen alte
Eingaben. 45 Scroll-, 117 Reader- und 622 Mac-Prüfbedingungen auch unter
ASan/UBSan: 39 eigene C-Dateien/56 Hashes; externe UI nicht instrumentiert,
kein Leak-/ThreadSanitizer. 81/81 lokale Release-Prüfungen bestehen (437,82 s).
Reader-/Kontextbilder betrachtet, Paket/Installation geprüft.
[Vertrag](../../docs/SCROLLEN_BEIM_LADEN.md), Originale unter
`build/reader-scroll-verification/` und `build/reader-scroll-sanitizers/`.

0.9.60 korrigiert typisierte Cache-Adressen und strenge C17-Compilerprüfung.
Fensterprüfer bestätigen tatsächliche Größen; native Stilprüfer wählen den
Dokumentbereich, Exportvergleich prüft veröffentlichte Felder statt C-Padding.
80/80 lokal und alle vier Windows-/Linux-Jobs zu b6490bd bestehen.
[Vertrag](../../docs/UI_PLATTFORMKORREKTUREN.md).

Hilfe kehrt mit Eingaben, Auswahl, Fokus, Meldung und Scrollposition zur
Ursprungsaufgabe zurück. Gewöhnliche Modalaufgaben erhalten Reader-/Editor-
Position und gehaltene Quellen-/Glyphen-/Inline-/Stilpläne. Native Kontexte
bleiben abgeschirmt. [Hilfe](../../docs/HILFE_RUECKKEHR.md),
[Reader-Rückkehr](../../docs/READER_RUECKKEHR.md).

## Nächste Arbeiten und verbleibende Abnahmen

0.9.62 begrenzt den visuellen Fokus lokal auf die aktive Modalgruppe.
310 Bedingungen, fünf gezielte Tests und ASan/UBSan bestehen; beide Bilder
angesehen. 82/82 Release-Prüfungen bestehen (413,41s); Paket/Installation und neue Zielplattformen
folgen. Installiert bleibt vorerst0.9.61. [Vertrag](../../docs/MODALER_FOKUS.md).

Erste Quell-/Parser-/Worker-Aufbereitung, native Veröffentlichung, Tabellen,
Rasterung, Budgetrückfälle, riesige Absätze und Shutdown weiter profilieren.
SBUI-074 bleibt für vollständige Leistungs-/Geräte-/Großdatenabnahme offen.
Native Snapshot-Pixelidentität und Material-Ersatzfläche bei Speicherfehler offen.

Tatsächliche IME/VoiceOver/NVDA/Orca, native Dialoge, Geräte/Monitore/Langzeit,
Windows-/Linux-Zielvolumes und physische Persistenz, Mindest-OS/frische Rechner,
vollständige Runtime-/Lizenzprüfung, vertraulicher Sicherheitskanal, Signierung,
dauerhafte aktuelle Release-Pakete und abschließende natürliche Texte bleiben
Release-Arbeit. Apple-Developer-Konto und Windows-Signaturzertifikat fehlen.
**1.0 nur nach ausdrücklicher Nutzerfreigabe.**
