# Beschriftungen kürzen und verdeckte Bedienelemente erhalten

Stand: 8. Oktober 2026, Entwicklungsschritt 0.9.38. Dieser Schritt behebt einen
zusätzlichen Aufwand beim ersten Layout und zwei Darstellungsgrenzen. Er löst
noch nicht die gesamte Verzögerung langer Dokumente; [Nachweise](STATUS.md).

## Kürzung

Die bisherige Kürzung entfernte einzelne Unicode-Skalarwerte und maß jeden
neuen Teilstring. Lange unterschiedliche Beschriftungen erzeugten dadurch viele
vollständige Schriftmessungen. Die neue Funktion verwendet die gemeinsame
[Textmessung](PLAIN_TEXT.md) und sucht an ganzen Graphemgrenzen. Die vollständige
Anzeige aus Präfix und Auslassungszeichen wird danach nochmals gemessen, damit
Kerning und Schrift-/Richtungsauflösung in diesem tatsächlichen String passen.

Auch feste Puffer für Tastatur-/Screenreaderbeschriftungen und Hinweise enden
jetzt an Graphemgrenzen. Ein Linklabel wird bis zum Erzeugen des Buttons vollständig
gehalten; es wird nicht bereits vorher auf ein möglicherweise halbes Emoji gekürzt.
Markdown und gespeicherte Texte bleiben unverändert.

## Verdeckte Buttons

Buttons außerhalb ihres Panels behalten ihre logischen Tastaturziele,
Beschriftungen und Aktivierung. Iconzeichnung und visuelle Textkürzung entfallen
für diese vollständig verdeckten Flächen. Ein Hinweis darf nur entstehen, wenn
die Maus sowohl über dem Button als auch innerhalb seines Panelclips liegt.
Dadurch kann ein weggescrollter Link keinen Hinweis über einer anderen Fläche
anzeigen. Teilweise sichtbare Buttons werden weiterhin am vorhandenen Clip gezeichnet.

## Prüfung und verbleibende Leistung

Eine echte Desktopprüfung verwendet lange Familien-Emoji-Linklabels, einen
verdeckten Link und dessen Aktivierung ohne Maus. Anzeige- und Zielpuffer müssen
auf vollständigen Graphemen enden; verdeckter Text darf kein Textkommando erzeugen.
Nach Scrollen darf der verdeckte Link keinen Hoverhinweis beanspruchen. Quelle und
Entwurf bleiben unverändert. Gesamt-/Sanitizerergebnisse folgen in STATUS.

Ein instrumentierter Vergleich desselben Fontsystems kürzt 80 unterschiedliche
lange ASCII-Beschriftungen auf 90 Einheiten: 19.500 → 1.040 Breitenabfragen und
331,084 → 14,776 ms in dieser lokalen Stichprobe. Dies ist eine Messung dieser
Teilfunktion, keine allgemeine Bildraten- oder Dokumentwechselbehauptung.

Die kalte Vorbereitung des großen Journaldokuments bleibt auf dem UI-Thread
teuer. Zusätzliche genaue Messwege und spätere Glyphenerzeugung wurden versuchsweise
geprüft; sie brachten in der wechselnden nativen Last keinen belastbaren Zeitgewinn
und sind nicht Teil dieser Änderung. Ein abwechselnder Softwarevergleich zeigt
98,68 beziehungsweise 98,44 ms Median des kalten Layouts. Dafür bleibt eine eigene
Vorbereitungsstrategie erforderlich, die Schrift-Threadbesitz, Abbruch, geänderte
Dateien und unveränderte Scroll-/Zeilengeometrie berücksichtigt.
