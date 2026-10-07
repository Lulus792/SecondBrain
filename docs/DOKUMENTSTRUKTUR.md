# Struktur der Leseansicht

Stand: 7. Oktober 2026. Die Struktur ab 0.9.1 und die Linux-Korrektur ab 0.9.2
sind nativ nachgeprüft; 0.9.4 erweitert die eigenen [Blockregeln](MARKDOWN.md).
Die neue Plattformabnahme folgt nach den lokalen Prüfungen gesondert.

## Grundlage

Apple [Accessibility](https://developer.apple.com/design/human-interface-guidelines/accessibility),
öffentliche Dokumentationsdaten am 7. Oktober erneut gelesen: Inhalte und
Bedienwege müssen über die zugänglichen Schnittstellen erkennbar sein.
Die W3C-[Headings-Empfehlung](https://www.w3.org/WAI/tutorials/page-structure/headings/)
erklärt Ebenen und Abschnittsnavigation für Webinhalte. Die Übertragung auf
unsere Desktop-Leseansicht ist eine eigene Entscheidung; keine formelle
WCAG-Konformität wird daraus behauptet.

Die ATX-Regel für ein bis sechs führende Hashzeichen wurde am Original
[CommonMark 0.31.2](https://spec.commonmark.org/0.31.2/#atx-headings) geprüft.
Die bestehende Darstellung ist weiterhin eine begrenzte eigene Markdown-
Ansicht, keine vollständige CommonMark-Implementierung.

## Umsetzung

Die Leseansicht veröffentlicht ihren tatsächlichen Inhalt als Dokument mit
Überschriften, Absätzen, Codezeilen und Linkaktionen. Ab 0.9.6 liegen
[Tabellen](TABELLEN.md), Zeilen und Spaltenköpfe/Zellen darunter; die native
Matrixschnittstelle ist damit noch nicht auf allen Plattformen verfügbar. Der Dokumenttitel gehört
als erste Überschrift zur Struktur, auch wenn er im festen Kartenkopf steht.
Ebenen entsprechen den erkannten Markdown-Überschriften; sieben Hashzeichen
werden als Text behandelt. Fenced- und eingerückter Code werden als Code ausgewiesen. Die Erkennung
berücksichtigt ab 0.9.4 Zeichen, Länge und Einrückung der Codezäune; scheinbare
Überschriften und Links innerhalb von Code bleiben Text. Absätze behalten ihren vollständigen Text,
auch wenn sie länger als der aktuelle Bildausschnitt sind.

Textläufe liegen unter ihren jeweiligen Blöcken. Bei strukturierter Darstellung
wird kein zusätzlicher kompletter Markdown-Textlauf unter dem Dokument erzeugt.
Der ursprüngliche Dokumentwert bleibt für die bestehenden Schnittstellen
verfügbar. Native Textbereiche liefern den aufbereiteten Lesetext. Editorwerte
bleiben unverändertes Markdown und erhalten noch keine Blockstruktur.

Die eigene Anbindung führt Ebenen als 1–6; AccessKit C 0.23.1 erwartet sie
nullbasiert. Diese Umrechnung erfolgt beim Aufbau des Adapterbaums. Die
Windows-Level-Eigenschaft rechnet wieder auf natürliche Ebenen um; konkrete
UIA-/AT-SPI-Abfragen werden erst nach dem jeweiligen Lauf als bestanden genannt.
Die macOS-Prüfung bestätigt Überschriftenrollen und Textbereiche. Eine native
macOS-Ausgabe der Ebenennummern ist damit noch nicht belegt.

## Navigation und Schutz

Bei Fokus auf der Leseansicht wechseln Alt+Bild auf/ab die Überschrift. Weitere
Eingaben wechseln das Ziel schon während einer Bewegung. Reduzierte Bewegung
überspringt den Übergang. Normales Scrollen löst die zuvor gewählte Gliederungs-
position. Die üblichen Editor- und Sternkartentasten bleiben eigenständig.

Native Scroll-into-view-Anfragen gehen an dieselben, aus Quellpositionen
abgeleiteten Blöcke. Ihre Position wird aus dem aktuellen Layout ermittelt.
Dokument- oder Aufgabenwechsel verwerfen veraltete Anfragen. Abschnittssprünge
bearbeiten und speichern keinen Inhalt. Die Absätze sind keine zusätzlichen
Tabulatorziele; native Lese- und Absatznavigation bleibt vom App-Fokus getrennt.

Die Kennungsverwaltung verwendet eine Hash-Tabelle. Die Veröffentlichungs-
reihenfolge wird nach dem tatsächlichen UI-Aufbau sortiert. Dies vermeidet die
bisherigen linearen Kennungssuchen und Einfügesortierung bei vielen Blöcken.
Ein einzelner Großtest ist dennoch kein Nachweis für flüssige Dauerbenutzung
mit großen realen Wissensbasen.

## Nachweise und offene Arbeit

Lokale Providerprüfungen bestätigen neue Rollen, vollständigen Text über
6.000 Zeichen ohne doppelten Markdown-Lauf sowie echte native Abschnitts-
sprünge auf macOS. Tastaturziele, Sichtbarkeit, vollständige lange Absätze und
Modalabschirmung werden im selben Ablauf geprüft. Konkrete abschließende
Ergebnisse stehen im [Umsetzungsstand](STATUS.md).

Ab 0.9.17 gruppieren [native Listen und Zitate](CONTAINER_UI.md) ihre Blätter;
Tabellenmatrixschnittstellen, vollständige Markdown-Regeln, graphemgenaue Text-
geometrie, gemischte Schreibrichtungen, IME und reale Screenreader-Bedienung
bleiben weitere Arbeiten. Blockrechtecke sind noch keine Zeichenrechtecke.
Eine korrekte Baumstruktur allein belegt keine vollständige assistive Abnahme.

## Linux-Nachprüfung

Der erste native 0.9.1-Lauf scheitert am fehlenden AT-SPI-`level`-Attribut
des vorhandenen Adapters. 0.9.2 bereitet eine gehashte UI-Quellkorrektur in
accesskit_atspi_common 0.21.0 vor; Typprüfung und idempotente Vorbereitung
bestehen lokal. Die tatsächliche AT-SPI-Abfrage und beide nativen Linux-Desktopprüfungen
bestehen inzwischen im [Wiederholungslauf zu 1358794](https://github.com/Lulus792/SecondBrain/actions/runs/37549092934).
Der frühere fehlerhafte Linux-Lauf bleibt als solcher dokumentiert.


## Abschnittstrennungen ab 0.9.10

[Trennlinien](TRENNLINIEN.md) werden als beschriftete horizontale Separatoren
veröffentlicht, ohne Textwert oder zusätzlichen Tabstopp. Ihre nativen Rechtecke
entsprechen der tatsächlichen Linie. Code und Setext-Unterstreichungen bleiben
eigenständige Fälle. Die lokale macOS-Rollen-/Geometrie-/Kontrastprüfung besteht;
eue Windows-/Linux-Nachweise werden erst nach tatsächlichem Lauf übernommen.
