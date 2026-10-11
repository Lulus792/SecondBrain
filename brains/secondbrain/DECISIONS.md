# SecondBrain Entscheidungen

Stand: 12. Oktober 2026. Nutzerentscheidungen und Implementierungsentscheidungen
sind getrennt benannt. Die [detaillierte historische Fassung](archive/decisions-2026-10-12.md)
bleibt erhalten; aktuelle Prüfgrenzen stehen in [STATE.md](STATE.md).

## D01: Eigene Anwendung in C auf drei Plattformen

Nutzerentscheidung: eigene C17-App für Windows, macOS und Linux; fachlicher Kern mit Standard-/OS-APIs, externe Bibliotheken ausschließlich UI.
[Maßgebliche Quelle](../../docs/PROJEKTPLAN.md).

## D02: Gemeinsames, lesbares Projektwissen

Nutzerentscheidung: Mensch und KI verwenden dieselben lesbaren Projektdateien. UTF-8-Markdown, JSON-Metadaten und kopierbarer Kontext; kein festgelegter KI-Anbieter.
[Maßgebliche Quelle](../../docs/KONZEPT.md).

## D03: UI-Recherche vor Gestaltung

Nutzerentscheidung: UI-Grundlagen und Apple HIG vor dem Entwurf recherchieren. Erkenntnisse und eigene Übertragung dokumentieren.
[Maßgebliche Quelle](../../docs/UI_RECHERCHE.md).

## D04: SDL3 und Nuklear ausschließlich in der UI

Implementierungsentscheidung: SDL3/Nuklear bilden die eigene Oberfläche; Textformung und Zugänglichkeitsbibliotheken gehören ebenfalls nur zur UI. Der Kern bleibt unabhängig.
[Maßgebliche Quelle](../../docs/ARCHITEKTUR.md).

## D05: Bewusst speichern und Wissen erhalten

Implementierungsentscheidung: bewusste Speicherung mit gelesener Revision, atomarem Ersetzen, Save-Guard und Konfliktkopie; erkannte fremde Inhalte erhalten.
[Maßgebliche Quelle](../../docs/DATENVERTRAG.md).

## D06: Geprüfte Schritte auf GitHub

Nutzerentscheidung: jeden abgeschlossenen, geprüften Schritt committen und ohne Force-Push veröffentlichen. Nur ausgeführte Prüfungen als Plattformnachweise nennen.
[Maßgebliche Quelle](../../AGENTS.md).

## D07: Gestaltung der räumlichen Wissensansicht

Nutzerpräferenz: dunkel/elegant, dezente Farben, kleine leuchtende Sterne und klare Beschriftungen; wenige schwebende Elemente.
[Maßgebliche Quelle](../../docs/UI_GALAXIE.md).

## D08: Lumen-Farben mit Liquid-Glass-Material

Nutzerwahl: Lumen-Farben und Liquid-Glass-Anmutung. Eigener plattformübergreifender Materialrenderer; keine native Apple-Materialkomponente.
[Maßgebliche Quelle](../../docs/UI_GALAXIE.md).

## D09: Vollständige Tastaturwege im neuen Design

Nutzerauftrag: vollständige Tastaturwege, sichtbarer Fokus, Tab/F6-Gruppen sowie erreichbare Dialoge, Quellen, Kontext und Bearbeitung.
[Maßgebliche Quelle](../../docs/UI_TASTATUR.md).

## D10: Direkte Navigation und kurze Übergänge

Nutzerauftrag: Pfeile öffnen direkt die nächste Notiz; kurze unterbrechbare Kamera-/Scrollbewegungen mit reduziertem Bewegungsmodus. Neutrale Beispiele, Schlussredaktion erst vor dem fertigen Produkt.
[Maßgebliche Quelle](../../docs/INTERAKTION.md).

## D11: Vollständige Vorbereitung vor 1.0

Nutzerauftrag: vollständige Release-Vorbereitung und UI-Politur. Eigener Code MIT. 1.0 erst nach ausdrücklicher Freigabe; Signierungskonten/Zertifikate fehlen.
[Maßgebliche Quelle](../../docs/RELEASE_ABNAHME.md).

## D12: Inhaltsarchive und neue Wiederherstellungsziele

Implementierungsentscheidung: eigene versionierte Inhaltsarchive, SHA-256-Prüfung und Wiederherstellung in ein freies neues Ziel; bestehende Projekte/Archive erhalten.
[Maßgebliche Quelle](../../docs/SICHERUNG.md).

## D13: Native Zugänglichkeit ausschließlich in der UI

Implementierungsentscheidung: AccessKit als externe UI-Anbindung, eigene C-Snapshots und begrenzte Aktionen mit Kontextkennungen. Menschliche assistive Abnahme bleibt erforderlich.
[Maßgebliche Quelle](../../docs/BARRIEREFREIHEIT_PLAN.md).

## D14: Linux-UI-Cachekorrektur aus festgelegter Quelle

Implementierungsentscheidung: festgelegte AccessKit-Quellen mit überprüften UI-Korrekturen und Hashschutz; Rust/Cargo nur zum Bauen dieser UI-Abhängigkeit, nicht für normale Nutzung.
[Maßgebliche Quelle](../../third_party/README.md).

## D15: Eigene Auswahl und wirksame Systemdarstellung trennen

Implementierungsentscheidung: eigene Auswahl speichern, wirksame Systemdarstellung getrennt anwenden. Portalabfragen außerhalb des UI-Threads; unbekannte Werte ersetzen keinen gültigen Stand.
[Maßgebliche Quelle](../../docs/EINSTELLUNGEN.md).

## D16: Einstieg aus dem tatsächlichen Projektzustand

Implementierungsentscheidung: erster Einstieg aus tatsächlichem Projektzustand, direkte fachliche Wege und Rückkehrfokus; kein verpflichtendes Tutorial.
[Maßgebliche Quelle](../../docs/ERSTER_START.md).

## D17: Alte Metadaten erhalten und unbekannte Schemas abweisen

Implementierungsentscheidung: Minimalmetadaten erhalten, Schema 1 unterstützen und unbekannte Schemas ablehnen. Keine automatische Vorlagenmigration; UTF-8 ohne NUL.
[Maßgebliche Quelle](../../docs/DATENVERTRAG.md).

## D18: Fehler je Projekt statt globaler Sperre

Implementierungsentscheidung: Fehler je Projekt mit Grund/Pfad anzeigen, andere gültige Projekte öffnen und erneutes Prüfen ermöglichen; Dateien nicht automatisch reparieren.
[Maßgebliche Quelle](../../docs/DATENVERTRAG.md).

## D19: Metadaten nicht allein beim Öffnen vertrauen

Implementierungsentscheidung: aktuelle Metadaten und Revision vor Schreibaktionen erneut prüfen. Keine globale Transaktionssperre; Datei und Entwurf bei erkannten Fehlern erhalten.
[Maßgebliche Quelle](../../docs/DATENVERTRAG.md).

## D20: Vorabversion erst nach vollständiger Paketabnahme veröffentlichen

Implementierungsentscheidung: dauerhafte Vorabversion erst nach vier nativen Paketabnahmen, Versionsgleichheit und verifizierten Prüfsummen veröffentlichen; vorhandene Releases nicht ersetzen.
[Maßgebliche Quelle](../../docs/DISTRIBUTION.md).

## D21: Gemeinsame schreibgeschützte Dokumentblockregeln

Implementierungsentscheidung: gemeinsamer eigener C-Dokumentbaum mit Quellpositionen und schreibgeschützter Projektion für Titel, Reader und native Struktur. Dokumentierte Markdown-Teilmenge.
[Maßgebliche Quelle](../../docs/MARKDOWN.md).

## D22: Gemeinsame Inline-Verweise und geschützte Originale

Implementierungsentscheidung: gemeinsame Inline-/Linkregeln und begrenzte Suche/Strukturtiefe. Code-/Bildbeispiele erzeugen keine Beziehungen; Originalbytes bleiben erhalten.
[Maßgebliche Quelle](../../docs/INLINE_LINKS.md).
