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

Installiert: **0.9.40 / 5b56e3542b38**, sauberer Intel/macOS-Release-Build.
94 Dateien entsprechen dem geprüften entpackten Paket. Vorherige Version 0.9.39:
`/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.39-20261008-171309`.
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

## Schutzdialog (0.9.39)

Fester Titel/Schließen und feste Entscheidungen neben einem eigenen scrollbaren
Meldungskörper. Große Schrift, lange Konflikttexte und Tab ohne Scrollreset
sind mit 199 Desktop-Assertions und denselben 199 unter haltendem ASan/UBSan
geprüft. Gesamtlauf 63/64; alter Tastatur-Fokusvertrag angepasst, vollständiger
Nachlauf mit 158 Assertions bestanden. Alle 64 Prüfungen dadurch abgedeckt,
kein einzelner grüner 64/64-Lauf. Build ohne Tests besteht.
[Vertrag](../../docs/SCHUTZDIALOG.md), Details/Sanitizergrenzen in STATUS.
Intel/macOS-Paket und Installation bestehen: 126 Desktop-, 158 Tastatur-,
75 Sicherungs-Assertions, zwei Neustarts und CLI-/Runtime-/Lizenzprüfungen.
Neue CI 37789106642 auf a24beb5 zuletzt queued; weitere Zielsysteme offen.

## Wortbedienung (0.9.40)

Eigene Unicode-18-Default-Wortsegmente im vorhandenen Textcache: Option-Pfeile
auf Mac, Strg auf Windows/Linux, Shift-Auswahl und Doppelklick ohne Folge-Leerraum.
Mac-Command-Pfeile bedienen Zeilengrenzen; Key-up und aktuelle Klickposition
sind geprüft. 67/67 lokale Release-Prüfungen, 125.569 Kern- und 126 UI-Assertions
unter haltendem ASan/UBSan bestehen; Build ohne Tests ebenso.
[Vertrag](../../docs/WORTNAVIGATION.md); Scope und Grenzen in STATUS.
Intel/macOS-Paket und Installation bestehen: 126 Desktop-, 158 Tastatur-,
75 Sicherungs-Assertions, zwei Neustarts und CLI-/Runtime-/Lizenzprüfungen.
Neue CI37798054080 zu5b56e35 zuletzt queued; weitere Zielsysteme offen.
Die Wiederverwendung geformter Zeilen brachte im alternierenden Vergleich
keinen Layoutgewinn und wurde entfernt. Kalter langer Wechsel bleibt offen.

## Nächste Arbeiten

Erstes Layout langer Dokumente weiter optimieren. Native Zeichenrechtecke
an tatsächliche Glyphen-/Zeilenpläne anbinden; native Wort-/Unicode-Bedienung
und reale Eingabemethoden prüfen. Weitere offene Release-Arbeiten: menschliche
VoiceOver/NVDA/Orca-, Dialog-, Geräte-, Mehrmonitor-/Langzeitabnahme; volle
Windows-/Linux-Zielvolumes und physische Persistenz; frische Rechner und
OS-Mindestversionen; vollständige SDK-/Systemruntime-Zuordnung.
Die Details stehen in der Release-Liste und ihren Originalnachweisen.

Eigener Code MIT. Signaturkonten fehlen; vertraulicher Sicherheitskanal ist
angefragt. Vollständiger Auftrag bleibt aktiv. **1.0 erst nach ausdrücklicher
Freigabe**; abschließende Produkttext-Bereinigung erst nach Vollständigkeit.
