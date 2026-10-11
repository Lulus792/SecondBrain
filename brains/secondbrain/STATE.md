# SecondBrain: aktueller Stand

Stand:11.Oktober2026. Originale: [STATUS](../../docs/STATUS.md),
[Release-Liste](../../docs/RELEASE.md), [Quellen](SOURCES.md).
Historische Schritte und Messungen stehen in STATUS und im [Journal](journal/2026-10-11.md).

## Produkt und installierte App

Eigene C17-App für Mensch und KI: Projekte/Notizen anlegen, lesen, bearbeiten,
suchen, archivieren, sichern; Quellen lesen und Kontext kopieren. Lumen-Sternkarte,
Glaskarten, eigene Icons, direkte Pfeilnavigation, Kamerafahrt, Startfokus, Textcursor
und große Leseansicht sind implementiert. Originalbytes, Entwürfe und erkannte
Konflikte bleiben geschützt. Externe Bibliotheken nur UI; MIT.

Installiert **0.9.63/92ab260e4203**, sauberer Intel/macOS-Release-Build.
96 Dateien paketgleich;18 Gedächtnisdateien beim Öffnen unverändert.126 Desktop-,
167 Tastatur-,75 Sicherungsbedingungen, zwei Einstellungs-Neustarts, CLI-/Runtime-/
Lizenz-/Version bestehen. Tatsächliche App-Aufnahme angesehen. macOS14.6.1/23G93,x86_64.
Rückfall: `/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.62-20261011-215805`.

## Geprüfter Fortschritt

0.9.63 veröffentlicht während des Ladens den sichtbaren Status als Readerkind,
keine vorzeitigen vollständigen Markdown-Textläufe. Danach wird der komplette
Dokumentinhalt veröffentlicht; frühe Scrollbefehle und native Rollen bleiben.
Negative Regression bestätigt; vier gezielte Tests und82/82 lokale Release-Tests
bestehen (447,15s). ASan/UBSan:120 Reader-,45 Frühscroll- und622 Mac-Bedingungen,
39 eigene C-Dateien/56 Hashes; externe UI nicht instrumentiert, Leaks aus, kein TSan.
[Vertrag](../../docs/LADEZUSTAND_NATIVE.md), Originale `build/reader-preflight-verification/`.

Drei serielle Metal-Paare: erstes Bild78,28→62,62ms,77,29→61,68ms,78,77→64,67ms;
mediane Verbesserung20,00Prozent. Quelle/Kopie unverändert. Kein menschlicher
Trackpad-/Eingabenachweis; erste fertige Textanzeige nicht unabhängig gemessen.
Ein Inline-Auftragscache ohne verlässlichen Gewinn wurde entfernt.

0.9.62 begrenzt visuelle Fokusrahmen auf die aktive Modalgruppe.0.9.61 erhält
Scrollbefehle während des Ladens und begrenzt sie nach Messung der Inhaltshöhe.
Hilfe kehrt mit Eingaben, Auswahl, Fokus, Meldung und Position zur Ursprungsaufgabe
zurück; normale Modalaufgaben erhalten Reader-/Editorpläne. Verträge in SOURCES.

## Aktuelle Plattformnachweise

CI38169362014 zu22a8504: Windows-Release und Linux-Debug/Release erfolgreich;
beide Release-Pakete tatsächlich geprüft. Windows-Debug114558512596 scheiterte
bei der Hilfe-Kontextprüfung: zwei Bilder garantieren keinen fertigen Reader.
Der Prüfer wartet jetzt begrenzt auf Bereitschaft und behält sämtliche Höhen-/
Rückkehrassertionen. Hilfe/Reader lokal bestehen (17,26s),94 Hilfebedingungen auch
unter ASan/UBSan mit bestätigten Bibliotheks-/Helperhashes. Neue Windows-Debug-
Nachprüfung folgt. Installierter Produktcode entspricht weiter92ab260; nur Prüfer
geändert. Native Mac-CI zuletzt queued; lokale Intel-Prüfung separat. ARM64 und
frische Nutzerrechner offen. Frühere Erfolge ersetzen keine neue Prüfung.

## Verbleibende Release-Arbeit

Erste Quelle/Parser/Worker, native Veröffentlichung fertiger Blöcke, Rasterung,
Tabellen, Budgetrückfälle, riesige Absätze und Shutdown weiter prüfen. SBUI-074
bleibt für vollständige Leistungs-/Geräte-/Großdatenabnahme offen. Native Snapshot-
Pixelidentität und Material-Ersatzfläche bei Speicherfehler ebenfalls offen.

IME/VoiceOver/NVDA/Orca, native Dialoge, Monitore/Skalierungen/Langzeit, volle
Windows-/Linux-Zielvolumes und physische Persistenz, Mindest-OS/frische Rechner,
vollständige Runtime-/Lizenzprüfung, vertraulicher Sicherheitskanal, Signierung,
dauerhafte aktuelle Release-Pakete und abschließende natürliche Texte bleiben
Release-Arbeit. Mindest-OS-Präferenz erneut angefragt; weitere Arbeit unabhängig.
Apple-Developer-Konto und Windows-Zertifikat fehlen. **1.0 nur nach Nutzerfreigabe.**
