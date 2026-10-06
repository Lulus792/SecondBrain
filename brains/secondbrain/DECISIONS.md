# SecondBrain Entscheidungen

Stand: 6. Oktober 2026.

## D01: Eigene Anwendung in C auf drei Plattformen

Beschlossen durch den Nutzer. Der normale Arbeitsablauf gehört in die eigene App.
Der Kern verwendet C und Betriebssystem-APIs; externe Bibliotheken sind nur für UI
zulässig. Windows, macOS und Linux werden tatsächlich geprüft.
Beleg: [Projektplan](../../docs/PROJEKTPLAN.md).

## D02: Gemeinsames, lesbares Projektwissen

Beschlossen durch den Nutzer: Das System dient Mensch und KI gemeinsam.
Implementierungsentscheidung: UTF-8-Markdown und kleine JSON-Metadaten übernehmen
die bestehende Struktur. Der Kern exportiert gespeicherte Kerninformationen als
kopierbaren Kontext. Kein bestimmter KI-Anbieter ist dafür erforderlich.
Beleg: [Konzept](../../docs/KONZEPT.md), [Architektur](../../docs/ARCHITEKTUR.md).

## D03: UI-Recherche vor Gestaltung

Beschlossen durch den Nutzer. Apple ist die gestalterische Orientierung.
Originalquellen und abgeleitete Prinzipien wurden vor dem Entwurf dokumentiert.
Die Implementierung verwendet eigene Flächen, Noto-Schriften und Systemfenster.
Beleg: [UI-Recherche](../../docs/UI_RECHERCHE.md),
[UI-Entwurf](../../docs/UI_ENTWURF.md).

## D04: SDL3 und Nuklear ausschließlich in der UI

Implementierungsentscheidung. Beide passen zur C-Anwendung und den Zielplattformen.
Der unabhängige Kern enthält sie nicht. SDL wird statisch eingebunden; Nuklear wurde
für UTF-8-Paste und große Cursorpositionen korrigiert.
Grenze: keine native Screenreader-Anbindung in Version 0.1.
Beleg: [Architektur](../../docs/ARCHITEKTUR.md),
[Versionen und Lizenzen](../../third_party/README.md).

## D05: Bewusst speichern und Wissen erhalten

Implementierungsentscheidung. Speichern prüft den gelesenen Stand und ersetzt
Dateien atomar. Ungespeicherte Wechsel fragen nach Speichern, Verwerfen oder
Abbrechen. Bei externen Konflikten kann eine eigene Notizkopie angelegt werden.
Beleg: [Kern](../../src/sb.c), [Anwendungszustand](../../app/model.c),
[Bedienprüfung](../../app/self_test.c).

## D06: Geprüfte Schritte auf GitHub

Beschlossen durch den Nutzer. Schritte werden ohne erzwungene Pushes veröffentlicht.
Plattformnachweise nennen tatsächlich erfolgreiche Läufe. Pakettests starten die
wirklich entpackte App.
Beleg: [Projektanweisungen](../../AGENTS.md),
[Plattformnachweise](../../docs/PLATTFORMEN.md).
