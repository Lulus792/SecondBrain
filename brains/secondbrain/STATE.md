# SecondBrain Aktueller Stand

Stand: 6. Oktober 2026. Diese Übersicht ist eine datierte Zusammenfassung;
aktuelle Nachweise und Grenzen stehen im [Umsetzungsstand](../../docs/STATUS.md).

## Belegter Stand

- Grundlagen und Apple UI-Recherche wurden vor dem UI-Entwurf dokumentiert.
- Der eigene C17-Kern verwaltet Projekte, Markdown, Suche, Archiv und Kontext.
- Die eigene Desktop-App bietet Anlegen, Lesen, Bearbeiten, Speichern, Quellen
  und Projektwechsel mit Schutz ungespeicherter Änderungen und Speicherkonflikte.
- Der [Desktop-Lauf zu 07ca223](https://github.com/Lulus792/SecondBrain/actions/runs/37443300206)
  besteht auf Windows, macOS und Linux, einschließlich des vollständigen Bedienablaufs.
- Lokal bestehen alle vier C-/UI-Prüfungen auch mit AddressSanitizer und
  UndefinedBehaviorSanitizer auf Intel macOS 14.6.1.
- Ein tatsächliches macOS-Archiv besteht nach Entpacken und Verschieben denselben
  Bedienablauf. Das [Paketverfahren](../../docs/DISTRIBUTION.md) ist implementiert.
- Diese Wissensbasis wurde mit dem C-Kern angelegt und mit projektspezifischen
  Inhalten und relativen Originalquellen befüllt.

## Laufende Arbeit

Der [Paketlauf zu aeb1d0d](https://github.com/Lulus792/SecondBrain/actions/runs/37443877273)
besteht mit Desktop Debug/Release und den entpackten Paketen auf allen drei Systemen.
Die eigene Instanz ist in der App geladen und betrachtet; ihre lokalen Quellen
sind erreichbar. Der erweiterte Bedienablauf prüft jetzt zusätzlich Konfliktkopie,
Archivierung und Speichern beim Beenden.

## Nächster Schritt

Den nächsten GitHub-Lauf einschließlich eigener Projektinstanz und erweitertem
Bedienablauf auswerten. Danach die Abnahme und diesen Stand mit dem Ergebnis ergänzen.

## Grenzen

Keine native Screenreader-Anbindung, automatische Synchronisation oder integrierte
Chat-Anbieter. Grundlegende Markdown-Darstellung; Editor-Undo und Schriftabdeckung
sind begrenzt. Pakete sind nicht durch Apple notarisiert.
Details: [Distribution](../../docs/DISTRIBUTION.md).
