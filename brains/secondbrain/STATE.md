# SecondBrain: aktueller Stand

Stand: 8. Oktober 2026. Originale: [STATUS](../../docs/STATUS.md),
[Release-Aufgaben](../../docs/RELEASE.md), [Quellen](SOURCES.md).
Chronologie und frühere Nachweise: [Journal](journal/2026-10-08.md).

## Produkt und installierte App

Eigene C17-App für Mensch und KI: Projekte/Notizen verwalten, lesen, bearbeiten,
suchen, archivieren und sichern; Quellen und gespeicherten Kontext kopieren.
Lumen-Sternkarte, Glaskarten/Icons, direkte Pfeilnavigation, Kamerafahrt,
weiches Scrollen, Startfokus, Textcursor und große Leseansicht sind implementiert.
Originalbytes, Entwürfe und erkannte Konflikte bleiben geschützt.

Installiert: **0.9.37 / 47a5a4392418**, sauberer Intel/macOS-Release-Build.
94 Dateien entsprechen dem geprüften entpackten Paket. Vorherige Version 0.9.35:
`/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.35-20261008-134625`.
Installierte Ansicht betrachtet; Laden verändert keine Gedächtnisdatei.

## Geprüfter aktueller Fortschritt

Reader, Editor/Felder und einfache Texte verwenden gemeinsame Absatz-/Script-/
Glyphenpläne. Cursor, Auswahl, Maus, Pfeile und IME-Markierungen folgen derselben
Geometrie. Allgemeine Umbruchflächen behalten Absatzrichtung über Folgezeilen;
Höhe und Zeichnung teilen den Plan. Code bleibt wörtlicher Text. Ganze Grapheme,
CRLF und leere Absätze sind erhalten; Widgetclips und Cachepolitik sind geprüft.
[Editorvertrag](../../docs/EDITORGEOMETRIE.md),
[einfacher Text](../../docs/PLAIN_TEXT.md), [Umbruch](../../docs/WRAPPED_TEXT.md).

62/62 lokale Release-Prüfungen (294,76 s), 623 unabhängige Umbruch-/Glyphen-/
Pixel-Assertions unter haltendem ASan/UBSan und UI-Build ohne Tests bestehen.
Sanitizer-Scope und Grenzen nennt STATUS. Das entpackte Mac-Paket besteht
126 Desktop-, 157 Tastatur- und 75 Sicherungs-Assertions, zwei Neustarts,
Produktions-CLI und Runtime-/Lizenz-/Versionsprüfung.

0.9.36 besteht alle 20 Plattformjobs/vier Pakete (CI 37769203269).
Neue 0.9.37-CI 37771435451 besteht inzwischen alle 20 Jobs/vier Pakete.
Die tatsächlichen menschlichen/gerätebezogenen Abnahmen bleiben gesondert offen.

Native Metal-Stichprobe auf diesem Intel-Mac: Scroll-Median 1,88 ms,
Wechsel 14,00 ms/p95 22,91 ms; erster langer Wechsel 107,58 ms, davon
95,40 ms Layout. Keine allgemeine Bildraten- oder Langzeitabnahme.

## Beschriftungsfortschritt (0.9.38)

Verdeckte Buttons behalten Tastaturziele/Aktivierung ohne visuelle Textmessung.
Label-/Hinweispuffer und Kürzung erhalten ganze Grapheme. 63/63 lokale Release-
Prüfungen, 36 Desktop-Assertions unter haltendem ASan/UBSan und Build ohne Tests
bestehen. 80 lange Testlabels: 19.500 → 1.040 Breitenabfragen; der gesamte kalte
Dokumentwechsel bleibt im abwechselnden Softwarevergleich nahezu unverändert
(98,68 → 98,44 ms Layoutmedian). Neue Plattform-/Paketabnahme folgt;
installiert bleibt 0.9.37. [Vertrag](../../docs/BESCHRIFTUNGEN.md).

## Nächste Arbeiten

Erstes Layout langer Dokumente weiter optimieren. Native Zeichenrechtecke
an tatsächliche Glyphen-/Zeilenpläne anbinden; Unicode-Wortregeln und reale
Eingabemethoden prüfen. Weitere offene Release-Arbeiten: menschliche
VoiceOver/NVDA/Orca-, Dialog-, Geräte-, Mehrmonitor-/Langzeitabnahme; volle
Windows-/Linux-Zielvolumes und physische Persistenz; frische Rechner und
OS-Mindestversionen; vollständige SDK-/Systemruntime-Zuordnung.
Die Details stehen in der Release-Liste und ihren Originalnachweisen.

Eigener Code MIT. Signaturkonten fehlen; vertraulicher Sicherheitskanal ist
angefragt. Vollständiger Auftrag bleibt aktiv. **1.0 erst nach ausdrücklicher
Freigabe**; abschließende Produkttext-Bereinigung erst nach Vollständigkeit.
