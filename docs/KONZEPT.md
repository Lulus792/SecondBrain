# Konzept für ein wiederverwendbares Projektgedächtnis

Jedes Projekt erhält einen eigenen Ordner mit derselben Vorlage. Menschen finden
dort Orientierung und Wissen; die KI erhält einen nachvollziehbaren Einstieg
mit Verweisen auf die maßgeblichen Quellen. Der Kern bleibt klein genug, um ihn
bei der täglichen Arbeit aktuell zu halten.

Dieses Konzept beschreibt das Wissensmodell und die Arbeitsweise. Die geplante
eigene Anwendung in C sowie die Anforderungen an UI-Recherche und Gestaltung
stehen im [verbindlichen Projektplan](PROJEKTPLAN.md). Die dargestellten Dateien
gehören zum vorhandenen Strukturprototyp; die Oberfläche ist noch zu entwerfen.

## Aufbau einer Projektinstanz

| Datei oder Ordner | Aufgabe |
| --- | --- |
| START.md | Einstieg und Reihenfolge der wichtigsten Dokumente |
| PROJECT.md | Ziel, Umfang, Erfolgskriterien und Grenzen |
| STATE.md | Datierter Stand, laufende Arbeit und nächster Schritt |
| DECISIONS.md | Wesentliche Entscheidungen mit Gründen und Folgen |
| QUESTIONS.md | Offene Fragen, Blockaden und erforderliche Klärung |
| SOURCES.md | Originalquellen und ihre Zuständigkeit |
| inbox/ | Schnelles Erfassen ungeordneter Informationen |
| knowledge/ | Verdichtete Erkenntnisse mit Quellen und Projektbezug |
| journal/ | Übergaben und relevante Arbeitsabschnitte |
| archive/ | Inaktive Inhalte mit weiterhin nachvollziehbarer Herkunft |
| AGENTS.md | Pflegehinweise für KI-Arbeit innerhalb dieses Ordners |
| brain.json | Projektkennung, Erstellungsdatum und Vorlagenversion |

Die Vorlage enthält Hilfestellungen. Leere Bereiche sind ausdrücklich offen;
ein Generator kann ohne Projektwissen keine zuverlässige Projektbeschreibung
erzeugen.

## Quellen und Zusammenfassungen

SOURCES.md legt fest, welches Original welche Frage beantwortet. Bei Physim
liefert der Projektplan die Produktziele, docs/status.md den beschriebenen
Umsetzungsstand und docs/platform-validation.md die dokumentierten
Plattformnachweise. Die Anleitung steht in README.md und docs/guide.md.

Diese Rollen sind verschieden: Ein geplantes Merkmal ist keine bestätigte
Implementierung. Ein historischer Testbericht ist kein aktueller Gesamttest.
STATE.md bietet eine datierte Orientierung und verweist für Details zum Original.

Bei Widersprüchen werden beide Fundstellen und die offene Klärung notiert.
Die Zusammenfassung darf eine ungeklärte Abweichung nicht unbemerkt auflösen.
Änderungen am eigentlichen Projekt unterliegen dessen eigenen Anweisungen.

## Ein Arbeitsabschnitt

1. Auftrag und Stand lesen; Aktualität anhand relevanter Quellen prüfen.
2. Den nächsten Schritt mit Ergebnis und Erfolgskriterium bestimmen.
3. Während der Arbeit neue Hinweise schnell erfassen.
4. Am Ende wesentliche Erkenntnisse und Entscheidungen einordnen.
5. Stand, offene Fragen und nächsten Schritt aktualisieren.

Eine kurze Übergabe hält fest, was tatsächlich geändert oder geprüft wurde,
welche Einschränkungen gelten und wie jemand weiterarbeiten kann. Routine ohne
neuen Erkenntniswert benötigt keinen eigenen langen Journaleintrag.

## Gemeinsames Wissen zwischen Projekten

Zunächst bekommt jedes Projekt seinen eigenen Kontext. Wenn eine Erkenntnis in
mehreren Projekten gebraucht wird, kann sie später in einer gemeinsamen
Wissenssammlung stehen. Die Projekte verlinken dann auf dieselbe Notiz.
Projektbezogene Entscheidungen und Zustände bleiben in ihrer eigenen Instanz.

Beispiel: Allgemeine Grundlagen numerischer Fehler könnten gemeinsam genutzt
werden. Welche Genauigkeit Physim für ein bestimmtes Experiment fordert, gehört
weiterhin in den Physim-Kontext. So bleiben allgemeines Wissen und konkrete
Anforderungen unterscheidbar.

## Vorhandener Prototyp und geplante Anwendung

Der vorhandene Strukturprototyp umfasst die lokale Vorlage, einen Generator und ein
vorbereitetes Physim-Beispiel. Das Anlegen erfordert einen Befehl. Suche kann
im Prototyp über lokale Textsuche erfolgen.

Die geplante C-Anwendung übernimmt Projektverwaltung, Anzeige, Bearbeitung,
Erfassung und Suche selbst. Vor der Gestaltung wird UI-Design mit Orientierung
an Apple studiert. Die genaue KI-Anbindung und mögliche automatische Vorschläge
zur Pflege werden später konkretisiert. Automatische Änderungen benötigen einen
Abgleich mit den Quellen.

Wenn wir Vorlagen weiterentwickeln, bleibt vorhandenes Projektwissen erhalten.
Die Versionsnummer macht Unterschiede sichtbar. Eine spätere Migration muss
vorhandene Inhalte gezielt berücksichtigen; einfaches erneutes Erstellen wäre
dafür ungeeignet.

## Abnahme im Alltag

Ein neuer Chat erhält START.md und eine konkrete Aufgabe. Er sollte daraus den
Auftrag, den datierten Stand, geltende Entscheidungen und relevante Quellen
benennen können. Anschließend muss er die für die Aufgabe nötigen Quellen
prüfen können. Nach der Arbeit ist der nächste Einstieg weiterhin nachvollziehbar.

Das ist der wichtigste spätere Praxistest. Die Generatorprüfungen sichern
Dateierstellung und Bestandsschutz; sie bewerten nicht die Qualität des
eingetragenen Wissens oder das Verständnis einer KI.
