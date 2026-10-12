# Abstände der Sternbeschriftungen

Stand: 12. Oktober 2026. In der installierten 0.9.73 lag der Beginn einer
Beschriftung innerhalb des äußeren Fokusrings. Beim Wechsel nach links war der
Abstand durch die zusätzliche Textreserve noch kleiner. Ein fester vertikaler
Versatz passte sich nicht an größere Schrift an.

Die Entwicklungsfassung verwendet auf beiden Seiten den Rand des vollständigen
Textrechtecks und einen mit der Schrift skalierenden Abstand. Die tatsächlichen
Zeilenmetriken bestimmen die vertikale Mitte. Die bestehende Bevorzugung
ausgewählter Beschriftungen und die Clips vor den Glaskarten bleiben erhalten.

Apple empfiehlt konsistente Ausrichtung und Anpassung an größere Schrift;
die konkreten Maße sind unsere Produktentscheidung. Am 12. Oktober wurden
[Layout](https://developer.apple.com/design/human-interface-guidelines/layout)
und [Focus and selection](https://developer.apple.com/design/human-interface-guidelines/focus-and-selection)
über die offiziellen Dokumentationsdaten erneut im relevanten Umfang gelesen.

## Ausgeführte Prüfung

Intel/macOS 14.6.1, Release-Build auf f7a01ce99619 mit den beschriebenen
Änderungen: acht gezielte CTest-Gruppen bestehen in 102,37 Sekunden.
Der neue Test prüft 108 Bedingungen über zwölf Szenarien: 100/150/200 Prozent
Schrift, rechte/linke Beschriftung, Normal-/Kontrastdarstellung und Lesekarte.
Mit der bisherigen Darstellung scheitert die Abstandsprüfung reproduzierbar.
Endbilder bei 100 Prozent rechts sowie 150/200 Prozent links wurden betrachtet.
Die Bildkontrolle korrigierte zusätzlich eine Zwischenfassung, die lediglich
eine größere Zeichenfläche statt der tatsächlichen Schriftzeile zentrierte.

Geprüft wurden außerdem Vordergrundclips, Graph-Snapshot, Graphem-Ellipsen,
Bewegung, Suche/Textcache, Tastatur und Versionsausgabe. Quell- und Programmhashes
vor und nach dem abschließenden Lauf waren identisch. Originalnachweise:
`build/label-review/`; Review SBUI-087 unter
`/Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-12_04-50-23/UI_REVIEW.md`.

Die installierte App bleibt zunächst 0.9.73/15e63b4f7960. Ein neuer Paketstand,
Windows-/Linux-Ausführung, weitere Graphdichten, Geräte und menschliche assistive
Bedienung sind durch diesen Durchlauf nicht abgenommen. Keine 1.0-Freigabe.
