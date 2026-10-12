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

Beim ersten lokalen Durchlauf war noch 0.9.73/15e63b4f7960 installiert.

## Paket und Installation: 0.9.74

Der saubere Intel/macOS-Build **0.9.74 / eabf5b3ff114** ist installiert.
Nach der Versionsanhebung bestehen beide Versionsprüfungen und der normale
Rendererstart in 1,62 Sekunden. Die eigentliche Layoutkorrektur gehört zum
oben beschriebenen Acht-Test-Lauf; kein neuer vollständiger 90er-Lauf wird behauptet.

Das tatsächlich entpackte Paket besteht 126 Desktop-, 167 Tastatur- und 75
Sicherungsbedingungen, Einstellungen in zwei Prozessen, CLI, Ressourcen,
Lizenzen und Laufzeitprüfung. Normale Rendererwahl: Metal/Cocoa, sichtbares
Fenster, vier Frames und 394 beobachtete Module; 13 Testdateien unverändert.
170 Produktinputs und Programmidentitäten bleiben nach der Paketprüfung gleich.
Der echte **73→74→73-Wechsel** erhält 20 Testdateien und Einstellungen;
Sicherung und Wiederherstellung bestehen. Alle drei Updatebilder betrachtet.

98 installierte Dateien sind paketgleich. Installierte App-SHA256:
`d0b039034dce04ca8a5cf712bdc7858810040008aeb1715acb2dfc710fc65ff7`.
Archiv-SHA256:
`76b01067b3d3e28bdd64d3ec12c1d68bc3d9f490290c41d084da6d038fc6f4fb`.
Die installierte Ansicht wurde betrachtet. Beim Software-Snapshot und beim
zusätzlichen normalen Metal-Start bleiben alle 22 eigenen Gedächtnisdateien
unverändert. App-Metadaten: minos 14.0, SDK 15.2; tatsächlich ausgeführt auf
macOS 14.6.1, kein Nachweis auf macOS 14.0 oder ARM64.

Originale: `build/label74-verification/`, `build/label74-package-check/`,
`build/label74-update/`. Rückfallkopie:
`/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.73-20261012-050232`.
[CI38197916019](https://github.com/Lulus792/SecondBrain/actions/runs/38197916019)
zum Paketquellstand besteht unter Windows/Linux in Debug und Release mit jeweils
90 Tests und beiden Release-Paketen. Die Paket- und Nachweis-ZIPs stimmen mit
ihren GitHub-SHA256-Digests überein. App-Hashes stimmen mit den nativen Berichten
überein: Direct3D11/Windows, 52 Module; OpenGL/X11, 65 Module. Jeweils sichtbares
Fenster, vier Frames und 13 erhaltene Testdateien. Originale unter
`build/label74-native-evidence/`. Mac-Jobs warten auf Runner.
Weitere Graphdichten, Geräte, neue Plattformläufe und menschliche assistive
Bedienung bleiben offen. Keine 1.0-Freigabe.
