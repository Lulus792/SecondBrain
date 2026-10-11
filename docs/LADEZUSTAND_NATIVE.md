# Ladezustand und native Dokumentausgabe

Stand: 11. Oktober 2026, Korrektur0.9.63.

Während die Leseansicht noch „Dokument wird vorbereitet …“ zeigte, veröffentlichte
sie bereits den vollständigen Markdown-Quelltext als unstrukturierten nativen
Dokumentwert. Die Zugänglichkeitsanbindung zerlegte diese unsichtbare Quelle
synchron in Textläufe. Eine gezielte Metal-Diagnose misst etwa12–17ms dafür
bei langen Wechseln; Parser/Vorbereitungsaufträge lagen bei etwa3ms.
Die Bereiche sind verschachtelt und dürfen nicht als unabhängige Summe gelten.

Die native Ausgabe folgt jetzt dem sichtbaren Zustand: Der Ladehinweis ist ein
Kind des Readers; dessen Wert bleibt vorläufig leer. Name, Dokumentrolle und
Scrollaktionen bleiben vorhanden. Sobald das Layout bereit ist, wird der
vollständige bisherige Dokumentwert mit den gezeichneten Blöcken veröffentlicht.
Quelle, Entwurf, Linkauflösung und die gepufferten frühen Scrollbefehle bleiben
an ihren bisherigen Regeln. Es gibt keinen zusätzlichen Timer.

## Nachweise

Die Regression in `tests/test_reader_prepare.c` scheitert mit dem bisherigen
Code: Der native Baum enthält während des Ladens bereits „A paragraph with“.
Die korrigierte Prüfung kontrolliert Ladehinweis, Elternbeziehung, fehlenden
unfertigen Inhalt und vollständigen Inhalt nach Bereitschaft. Eine externe
Literalquelle erhält dieselbe Prüfung. Geometrie-, Resize-, Font-, Rückkehr-,
Abbruch- und native Plattformprüfungen bleiben eigenständige Abnahmen.

Drei serielle Metal-Paare vergleichen den eingefrorenen Desktopcode aus4cdab61
mit der Korrektur. Beide verwenden denselben Timingprüfer und dieselben UI-
Bibliotheken, 1336×840 auf Intel/macOS14.6.1. Kopie und18 Originaldateien bleiben
unverändert; keine gleichzeitigen Builds oder UI-Prüfer. Jeder Lauf enthält272
Bilder und Wechsel zwischen Übergabe7.Oktober, Eingang und START.

| Paar | Erstes Bild vorher → danach | Erstes Layout vorher → danach | Wechsel-Frame-p95 vorher → danach |
| --- | --- | --- | --- |
| 1 | 78,28 →62,62ms | 32,25 →15,62ms | 21,10 →18,61ms |
| 2 | 77,29 →61,68ms | 29,62 →15,30ms | 20,17 →20,41ms |
| 3 | 78,77 →64,67ms | 31,97 →16,19ms | 21,44 →18,22ms |

Median der paarweisen ersten Bildverbesserung etwa20Prozent. Der erste fertige
Text und menschliche Eingabeverzögerungen sind damit nicht unabhängig gemessen.
Wechsel-p95 verbessert sich nicht in jedem Paar. Kalte Schrift-/Rasterarbeit,
native Veröffentlichung fertiger Blöcke, schwächere Hardware und echte Trackpads
bleiben Leistungsarbeit; SBUI-074 und die Release-Liste bleiben offen.

Ein vorheriger Versuch, Inline-Analysen bereits bei der Worker-Auftragserfassung
zu halten, brachte in drei Paaren keinen verlässlichen ersten Bildgewinn. Er
wurde entfernt und ist kein Bestandteil0.9.63. Originale bleiben für Diagnose
unter `build/reader-preflight-verification/`, insbesondere `profiles/`,
`native-profiles/`, `stages/` und die Baseline-/Regressionsprotokolle.

Vier gezielte Tests bestehen (24,21s); die aktuelle Literalansicht mit großer
Schrift wurde angesehen. Gesamt-, Paket- und neue Zielplattformabnahme folgen. Kein Nachweis
menschlicher VoiceOver-/NVDA-/Orca-Bedienung und keine allgemeine Flüssigkeitszusage.

ASan/UBSan bestehen für120 Reader-,45 Frühscroll- und622 Mac-Providerbedingungen;
39 eigene C-Dateien/56 aktuelle Hashes geprüft. Externe UI nicht instrumentiert,
Leaks aus, kein ThreadSanitizer. Originale `build/reader-preflight-sanitizers/`.

0.9.63:82/82 lokale Release-Prüfungen bestehen (447,15s). Quellstand
22a8504ddb84e8c6a0428005697339dd78adda05 ist veröffentlicht. CI38169362014:
Linux-Release einschließlich Paket erfolgreich; andere Windows-/Linux-Desktopjobs
zuletzt in Arbeit, native Mac-Jobs queued. Sauberer Paketbuild folgt.

Paket0.9.63/92ab260e4203 ist lokal geprüft und installiert;96 Dateien paketgleich,
18 Gedächtnisdateien beim Öffnen unverändert, Aufnahme angesehen. Windows-Release
und beide Linux-Jobs zu22a8504 bestehen. Windows-Debug zeigte eine verfrühte
Hilfe-Kontextprüfung nach zwei Bildern. Der Prüfer wartet jetzt begrenzt auf die
fertige Ansicht; alle Höhen-/Rückkehrassertionen bleiben bestehen. Hilfe/Reader
lokal und94 Hilfebedingungen unter ASan/UBSan nachgeprüft, neuer Windows-Debug-
Nachweis folgt. Mac-CI zuletzt queued; keine menschliche assistive Abnahme.
