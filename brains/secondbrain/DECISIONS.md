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

## D07: Gestaltung der räumlichen Wissensansicht

Bestätigte Nutzerpräferenzen vom 6. Oktober 2026: dunkel und elegant mit
dezenten Farben; kleine leuchtende Sterne mit klaren Beschriftungen; wenige
schwebende Bedienelemente und Notizen bei Auswahl. Zusätzlich soll eine Variante
mit glasartigen Karten über der Galaxie gezeigt werden.

Lumen, Glas und Fokus waren die ersten Entwürfe zur Auswahl.
Beleg: [Designrecherche](../../docs/UI_GALAXIE.md).

## D08: Lumen-Farben mit Liquid-Glass-Material

Der Nutzer hat am 6. Oktober 2026 die Farbgestaltung von Lumen ausgewählt.
Karten und weitere Bedienelemente sollen einen ausgeprägten Liquid-Glass-Look
wie bei Apple erhalten. Die verfeinerte Designstudie zeigt diese Kombination
mit Hintergrundbrechung und reagierenden Lichtkanten.

Die Vorschau ist eine eigene optische Nachbildung. Ein Originalmaterial von
Apple ist damit noch nicht in die C-App integriert. Materialanbindung und
Plattformnachweise gehören zum nächsten Implementierungsschritt.
Beleg: [Verfeinerte Studie](../../docs/UI_GALAXIE.md).

## D09: Vollständige Tastaturwege im neuen Design

Am 6. Oktober hat der Nutzer die Umsetzung des neuen Designs und problemlose
Navigation ausschließlich per Tastatur beauftragt. Tab folgt einer sichtbaren,
stabilen Reihenfolge; F6 wechselt Gruppen. Die ursprüngliche getrennte Sternwahl
wurde durch den Folgeauftrag zur direkten Navigation ersetzt (D10).
Dialoge, Quellen, Kontext und Bearbeitung bleiben erreichbar. Ein eigener
SDL-Tastaturdurchlauf prüft die Abläufe ohne injizierte Mausereignisse.
Quelle: Nutzerauftrag; [Bedienvertrag](../../docs/UI_TASTATUR.md).

## D10: Direkte Navigation und kurze Übergänge

Der Folgeauftrag vom 6. Oktober verlangt sofortiges Öffnen der nächsten Notiz mit
Pfeiltasten und flüssigere Bewegungen. Kamera und Scrollposition verwenden kurze,
unterbrechbare Übergänge; reduzierte Bewegung ist in der App wählbar. Der Schutz
ungespeicherter Änderungen bleibt maßgeblich. Ein fremdes Projektbeispiel wurde
auf Nutzerwunsch vollständig aus der aktuellen Arbeitsfassung entfernt; neutrale
Testdaten ersetzen es. Die Git-Historie bleibt erhalten.

Die README orientiert sich nach Recherche an üblichen Desktop-Projekten. Eine
abschließende sprachliche Prüfung des Endprodukts folgt erst vor 1.0; die
Entwicklungsdokumentation bleibt erhalten. [Release-Aufgaben](../../docs/RELEASE.md)
und [README-Recherche](../../docs/README_RECHERCHE.md).
