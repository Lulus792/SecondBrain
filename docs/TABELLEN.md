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
GFM-Unterstützung einschließlich verschachtelter Container wird nicht zugesagt.

Die Darstellung wechselt unter 130 skalierten Pixeln je Spalte zu gestapelten
Zeilen. Alle Zellen benutzen denselben vertikalen Lesebereich. Die native
Struktur und Zellidentitäten bleiben beim Darstellungswechsel erhalten.
Linkaktionen bleiben über die vorhandene Fokusnavigation erreichbar.

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
