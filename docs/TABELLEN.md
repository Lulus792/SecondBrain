# Tabellen in der Leseansicht

Stand: 7. Oktober 2026. Die Grundlagen und der Entwurf wurden vor der Umsetzung
gelesen und dokumentiert. Entwicklungsschritt 0.9.6 ergänzt eigene C-Regeln für
Markdown-Tabellen, ihre Leseansicht und einen strukturierten Zugänglichkeitsbaum.

## Gelesene Grundlagen und eigener Entwurf

Apple [Lists and tables](https://developer.apple.com/design/human-interface-guidelines/lists-and-tables)
empfiehlt verständliche Spaltenüberschriften, lesbaren Text und erwägt wechselnde
Zeilenfarben zur Orientierung. Eigene Übertragung auf Dokumenttabellen: eine
ruhige Kopfzeile, dezente Datenzeilen, vollständiger Text und die bestehende
Schriftgröße. Tabellen im Dokument sind schreibgeschützt; Sortieren würde die
Quellreihenfolge verfälschen und gehört nicht zu dieser Ansicht.

Passt die Mindestbreite der Spalten nicht in die Karte, wird jede Datenzeile
vertikal mit ihren Spaltenbeschriftungen dargestellt. Diese eigene Anpassung
behält alle Werte und die native Tabellenstruktur. Es entsteht kein zweiter
horizontaler Scrollbereich. Verweise benutzen die vorhandenen Tastaturwege.

Die Syntax wurde an [GFM Tables](https://github.github.com/gfm/#tables-extension-)
geprüft: Kopf-/Trennzeile, Ausrichtung, maskierte Pipes und variable Datenzeilen.
Fehlende Datenzellen werden ergänzt; überschüssige Zellen folgen der GFM-
Darstellung. Die Quelldatei und der Editor behalten sämtliche Originalbytes.
Parser, Zellenaufbereitung, Verweise und Tests sind eigener C-Code.

Native Struktur: Dokument → Tabelle → Zeile → Spaltenkopf/Zelle → Text und
gegebenenfalls Linkaktionen. Indizes und Zeilen-/Spaltenzahlen werden mitgeführt.
Die konkrete native Provider-/Clientabnahme folgt der Implementierung; ein
richtiger Baum allein belegt keine vollständige Screenreader-Bedienung.

## Umfang und Grenzen

Kopf- und Trennzeile müssen gleich viele Spalten enthalten. Die Trennzellen
bestehen aus mindestens einem Bindestrich, optional mit Doppelpunkten für
linke, mittige oder rechte Ausrichtung. Äußere Pipes sind optional; mindestens
eine unmaskierte Pipe muss die Kopfzeile kennzeichnen. Maskierte Pipes werden
vor der Inline-Erkennung dekodiert, auch in Codezellen. Fehlende Datenzellen
bleiben leer; zusätzliche Werte erscheinen nicht in der Tabelle, bleiben aber
in Quelldatei und Editor erhalten. Codeblöcke erzeugen keine Tabellen.

Grenzen: höchstens 64 Spalten und 65.536 Zellen einschließlich Kopfzeile innerhalb
des bestehenden Textlimits. Eine nicht erkannte oder zu große Tabelle fällt auf
lesbaren Quelltext zurück. Die App lädt keine Bilder aus Zellen. Vollständige
GFM-Unterstützung wird nicht zugesagt. Ab 0.9.17 erkennt der gemeinsame
[Dokumentbaum](DOKUMENTBAUM.md) Tabellen auch in Listen/Zitaten; Zellreferenzen
verwenden dieselbe dokumentweite Umgebung.

Die Darstellung wechselt unter 130 skalierten Pixeln je Spalte zu gestapelten
Zeilen. Alle Zellen benutzen denselben vertikalen Lesebereich. Die native
Struktur und Zellidentitäten bleiben beim Darstellungswechsel erhalten.
Linkaktionen bleiben über die vorhandene Fokusnavigation erreichbar.

Ab 0.9.11 verwenden Zellen und gestapelte Kopftexte dieselben
[Inline-Stile](INLINE_STILE.md) wie Absätze und Überschriften. Messung, Ausrichtung
und Umbruch erhalten die sichtbare Hervorhebung und ganze Grapheme.

## Native Schnittstellen

AccessKit C 0.23.1 enthält Consumer 0.39.1 und macOS-Adapter 0.27.1.
Die [Consumer-Quelle](https://docs.rs/accesskit_consumer/0.39.1/src/accesskit_consumer/node.rs.html)
behandelt Dokumenttabellen nicht als auswählbare Container. Deshalb liefert der
[macOS-Adapter](https://docs.rs/accesskit_macos/0.27.1/src/accesskit_macos/node.rs.html)
für sie keine `accessibilityRows`. Die eigene C-Anbindung ergänzt diese Abfrage
und ihre Freigabe anhand der vorhandenen Tabellenkinder. Diese Ergänzung hängt
nicht vom erst in neueren AppKit-Versionen verfügbaren Überschriftenrollensymbol ab.

Die festgelegten Windows-/Linux-Provider veröffentlichen Rollen und Kinder,
aber noch kein UIA-GridPattern beziehungsweise AT-SPI-Table-Interface. Die native
Clientprüfung unterscheidet deshalb ausdrücklich die vorhandene Zeilen-/Zellen-
Hierarchie von einer Matrixabfrage. Eine erfolgreiche Baumprüfung schließt
weder diese Lücke noch die menschliche VoiceOver/NVDA/Orca-Abnahme. Vollständige
native Tabellennavigation bleibt eine offene Release-Aufgabe.

## Nachweise

Lokale Kernprüfung: 18 Tests bestanden (22,59 s). ASan/UBSan: abschließend 5.644 Assertions,
einschließlich 5.000 begrenzter Eingaben, bestanden. Die abschließenden UI-,
Paket- und Plattformprüfungen werden nach ihrem tatsächlichen Abschluss im
[Umsetzungsstand](STATUS.md) und den [Plattformnachweisen](PLATTFORMEN.md) ergänzt.


## Schmale Karte ab 0.9.7

Entwurf am 7. Oktober vor Umsetzung dokumentiert und anschließend implementiert:
In gestapelten Tabellen mit Datenzeilen
beginnt die Ansicht direkt mit den beschrifteten Werten. Eine zusätzliche reine
Kopfzeile ist dort redundant und beansprucht bei großer Schrift den ersten
Bildausschnitt. Die logischen Spaltenköpfe bleiben für native Leser erhalten;
ihre eigene, ausgeblendete Zeile bekommt kein sichtbares Rechteck. Ein Ansprung
führt zum Anfang der Daten und damit zu den wiederholten Beschriftungen.

Enthält die Kopfzeile Linkaktionen oder besitzt die Tabelle nur eine Kopfzeile,
bleibt sie sichtbar. Headerlinks müssen weiterhin über dieselben Tastaturwege
erreichbar sein. Leere Spaltenbeschriftungen erzeugen in Datenzeilen keine
zusätzliche Leerzeile. Dies ist eine eigene responsive Gestaltung; Apple schreibt
keine solche konkrete Umwandlung vor. Abnahme: kleine Karte bei 200 %, erste
Datenwerte sofort sichtbar, gleiche native Matrix und Identitäten, erhaltene
Headerlinks und Originalbytes; Spaltendarstellung unverändert lesbar.


Lokal bestehen fünf passende UI-/Kernprüfungen (34,07 s) sowie die nachfolgende
native Tab-/Providerprüfung. Mit der abschließenden Prüfwerkzeug-Anpassung
bestehen 270 native Assertions (26,67 s). Die Prüfung scheitert mit dem vorherigen
0.9.6-Renderer an der weiterhin sichtbaren Kopfzeile. Rasterbilder zeigen die
ersten Werte und den erhaltenen Kopfzeilenlink bei 780×560 und 200 %.
Die neue Windows-/Linux-/ARM64-Abnahme folgt nach dem Commit.

## macOS-Matrix, 8. Oktober 2026

Ab 0.9.27 liefert die native AXTable Zeilen-/Spaltenzahlen und direkte
Zellenabfragen. Die lokale Release-Prüfung `native-accessibility` besteht
(16,75 Sekunden); sie vergleicht alle zwölf Zellen einer 4×3-Tabelle mit
dem nativen Baum und prüft ungültige Indizes. Die Vorprüfung am alten
Getter scheiterte an der Zeilenzahl. Windows UIA Grid/Table und Linux
AT-SPI Table/TableCell bleiben offen; dies ist kein VoiceOver-Nutzertest.
Primärvertrag: [Apple NSAccessibilityProtocol](https://developer.apple.com/documentation/appkit/nsaccessibilityprotocol/accessibilitycell(forcolumn:row:)).

## Windows-Matrix, Entwicklungsschritt 0.9.29

Die eigene UI-Ergänzung im festgelegten AccessKit-Windows-Adapter stellt
Grid/GridItem sowie Table/TableItem bereit. Nullbasierte logische Indizes,
Spannen, Tabellenbezug und Spalten-/Zeilenheader verwenden die von C
veröffentlichten Metadaten. Leere und außerhalb des Ausschnitts liegende
Zellen bleiben abfragbar; ungültige Indizes werden abgewiesen. Die Methoden
verwenden die bestehenden Kontext-/Baumprüfungen, keine separate Kopie der
fachlichen Daten. Dimensionen und Zellindizes sind in die vorhandenen
UIA-Eigenschaftsänderungen eingebunden. Markdown-Dateien bleiben unverändert.

Vor Umsetzung erneut gelesen: Microsoft
[Grid](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementinggrid),
[GridItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementinggriditem),
[Table](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingtable) und
[TableItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingtableitem).
Die Rust-Ergänzung liegt ausschließlich in der erlaubten UI-Bibliothek;
Anwendung und fachlicher Kern bleiben C17. Originale Lizenzen und Cargo-
Paketversionen werden erhalten.

Original node.rs-SHA256: `160ff7058edfcb84b8651d4f0111e8839cd1e63c2f08bb309ef65c9e028efb67`.
Vorbereitete node.rs-SHA256: `3368a6cdfec3e48784404e37604c295a3ba239e8f16e8e358ce24023bebed877`.
Vorbereitung, unveränderte Wiederholung und Abweisung einer unbekannten
Quelle ohne Änderung ihrer Bytes sind lokal geprüft.

Die native Windows-Prüfung verlangt die Matrix jetzt verbindlich. Sie
vergleicht alle zwölf Zellen einer 4×3-Tabelle mit dem nativen Baum,
Koordinaten/Spannen, Tabellenidentität, drei echte Headerbeziehungen und
ungültige Indizes; danach erneut in schmaler Ansicht bei 200 Prozent Schrift.
Der bisher erfolgreiche reine Baumtest kann diese Prüfung nicht ersetzen.
Lokale Prüfung für das Windows-Ziel mit offizieller Rust-Toolchain und
festem Lockfile besteht; tatsächlicher Windows-Build und Ausführung folgen
über die native CI. AT-SPI-Matrix und menschliche Screenreader-Bedienung
bleiben offene Release-Arbeit.
