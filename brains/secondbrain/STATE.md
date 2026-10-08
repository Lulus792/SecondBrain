# SecondBrain: aktueller Stand

Stand: 8. Oktober 2026. Maßgebliche Originale: [STATUS](../../docs/STATUS.md),
[Release-Liste](../../docs/RELEASE.md), [Quellen](SOURCES.md).
Frühere Fortschritte und Abnahmen: [Journal](journal/2026-10-08.md).

## Produkt und installierte App

Eigene C17-App für Mensch und KI: Projekte/Notizen anlegen, lesen, bearbeiten,
suchen, archivieren und sichern; Quellen und gespeicherten Kontext kopieren.
Lumen-Sternkarte, Glaskarten/Icons, direkte Pfeilnavigation, Kamerafahrt,
weiches Scrollen, Startfokus, Textcursor und große Leseansicht sind implementiert.
Originalbytes, Entwürfe und erkannte Konflikte bleiben geschützt.

Installiert: **0.9.43 / fe2436a7e57a**, sauberer Intel/macOS-Release-Build.
94 Dateien entsprechen dem geprüften Paket, Gedächtnis beim Laden unverändert.
Vorige 0.9.42: `/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.42-20261008-190720`.
Paketprüfung: 126 Desktop-, 158 Tastatur-, 75 Sicherungs-Assertions, zwei
Neustarts, produktive CLI sowie Runtime-/Lizenz-/Version.
CI37813553485 zufe2436a zuletzt queued; kein neuer Gesamt-Plattformnachweis.

## Aktueller geprüfter Fortschritt (0.9.43)

Native Editor-Metadaten werden bei exakt unverändertem Quellen-/Schrift-/
Zeilenplan mit eigenem 32-MiB-Budget wiederverwendet. Ausgaben besitzen ihre
Laufarrays und übernehmen die aktuelle Scrollposition. Über dem Budget bleiben
Quellen und Geometrie vollständig. [Vertrag](../../docs/NATIVE_TEXTGEOMETRIE.md).

68/68 lokale Release-Prüfungen (684,64s), 13.708 Editor-/Cache-, 405 native
Text- und 590 Providerprüfungen bestehen, dieselben drei Prüfer auch unter
haltendem ASan/UBSan. Scope/Hashnachweise in STATUS. Build ohne Tests besteht.
Vier abwechselnde lokale Profilpaare: nativer Export des vorbereiteten
67-KB-Editors 34,53→6,71ms; keine allgemeine Bildraten-/Kaltlayout-Abnahme.
Sauberes Intel/macOS-Paket und Installation bestehen; neue Zielplattformprüfungen
bleiben offen. Die oben genannten Paketprüfungen gehören zu0.9.43.

## Neue Diagnose des langen Wechsels

Ein reiner Breitenmessversuch erhielt Geometrie, zeigte im alternierenden
Vergleich keinen Gewinn (75,32→85,14ms Layoutmedian) und wurde vollständig
entfernt. Produktcode/Installation bleiben0.9.43. Der
[Vorbereitungsvertrag](../../docs/READER_VORBEREITUNG.md) folgt neu gelesenen
Apple Loading/Motion und dem festgelegten SDL_ttf-Fontthreadvertrag.
Neue UI-Diagnose SBUI-074; die Vorbereitung ist noch nicht integriert.

## Geprüfter Vorbereitungsunterbau (0.9.44)

Unveränderliche Fontdaten, private Worker-Fonts und eigener Quelltext sind
umgesetzt. Vor einmaliger Übernahme werden Source/Kontext/Fontbindung geprüft.
69/69 lokale Release-Prüfungen (596,74s),393 neue Job-/Fontprüfungen bestehen.
Dieselben393 plus405 native Text-,13.708 Editor- und590 Providerprüfungen auch
unter haltendemASan/UBSan; Scope/Hashes in STATUS. Build ohne Tests besteht.
[Vertrag](../../docs/FONT_SNAPSHOTS.md). Paket/Zielplattformen folgen.
Der produktive Wechsel ist noch nicht angebunden; das Ergebnis ist bislang
eine ungebrochene Zeile. SBUI-074 bleibt offen.

## Nächste Arbeiten

Vorbereitung auf vollständigen Umbruch/Styles/Blöcke erweitern, Job-/Output-/
Speichergrenzen und nichtblockierende Retireverwaltung prüfen und in den langen
Wechsel integrieren. Native Zeichenrechtecke für weitere
Felder und Leseblöcke integrieren.
Das erste Layout langer Dokumente und große Exporte über der Cachegrenze
weiter optimieren. Reale Eingabemethoden, VoiceOver/NVDA/Orca, Dialoge,
Geräte/mehrere Monitore/lange Sitzungen und X11-/Wayland-HiDPI prüfen.
Weitere Release-Abnahmen: volle Windows-/Linux-Zielvolumes und physische
Persistenz, frische Rechner/OS-Mindestversionen, komplette SDK-/Systemruntime-
Zuordnung. Die Release-Liste hält Umfang und Nachweise fest.

Eigener Code MIT. Signaturkonten fehlen; vertraulicher Sicherheitskanal ist
angefragt. Vollständiger Auftrag aktiv. **1.0 erst nach ausdrücklicher Freigabe**;
abschließende Produkttext-Bereinigung erst nach Vollständigkeit.
