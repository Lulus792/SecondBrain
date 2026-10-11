# Zur langen Notiz zurückkehren

Stand: 11. Oktober 2026, Umsetzung ab 0.9.58. UI-Befund SBUI-079.
78/78 lokale Release-Prüfungen bestehen (651,31 s); Paketabnahme folgt.
[UI-Nachprüfung](/Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-11_18-36-07/UI_REVIEW.md).

## Reproduzierter Fehler

Ein gewöhnliches Menü wie Aktionen ersetzte die Readeraufgabe, obwohl sich der
Notizinhalt nicht geändert hatte. Der bisherige Formkontext machte die laufende
Vorbereitung ungültig; Quelle und Worker wurden am Frame-Ende freigegeben.
Die Gegenprobe in 0.9.57 zeigt Parserzähler 1 vor dem Menü, 6 nach fünf Menübildern
und 7 nach Schließen. Quelle und Vorbereitung fehlen während des Menüs;
anschließend erscheint erneut die Vorbereitungsmeldung.

Der Eintritt in eine Modalaufgabe setzte außerdem alle Scrollslots zurück.
Die erste Korrektur erhielt den numerischen Lesewert, ließ im Hintergrund aber
eine zusätzliche H1 entstehen. Dadurch verschob sich der sichtbare Ausschnitt.
Ein unveränderter Scrollwert allein war kein ausreichender Nachweis.

## Inhalts- und Aktionskontext

Die Glyphen-Vorbereitung erhält eine Inhaltsidentität. Gewöhnliche Formulare,
Menüs und der Schutzdialog verändern sie nicht. Projekt, Generation, lokale
Quelle sowie der Wechsel zu einem tatsächlichen modalen Reader bleiben
Bestandteile. Textbytes, Schrift-Epoche, Breite, Skalierung und Pixeldichte
werden weiterhin auf passende Vorbereitung geprüft.

Der native Aktionskontext bleibt separat. Öffnen eines Dialogs sperrt weiterhin
den Hintergrund; dessen aktive Ziele/Metadaten werden vor der nativen
Veröffentlichung entfernt. Gehaltene Glyphen erlauben keinen Zugriff auf
verdeckte Editor- oder Readeraktionen.

Normale Kontrollaufgaben behalten Quelle, Inline-/Stilpläne und den fertigen
Reader-Worker. Ein modaler Kontext-/Lizenzreader besitzt weiterhin seine eigene
Vordergrundaufgabe und ersetzt den anderen gehaltenen Reader. Der feste
Notiztitel wird unter gewöhnlichen Formularen nicht zusätzlich in den Lesetext
eingefügt. Erstmalige Vorbereitung, echte Quellen-/Font-/Breitenwechsel,
Budgetrückfälle und Bildausgabe bleiben eigene Leistungsfälle.

Ein neuer Dialogkörper startet mit seinem eigenen Scrollzustand. Reader und
Editor behalten Position, Bereich und Inhaltshöhe. Ein tatsächlich neu
geöffneter modaler Reader startet getrennt. Fokuswechsel innerhalb derselben
Modalaufgabe setzen den Körper nicht erneut zurück. Nach Abbruch greift die
bestehende Fokusrückkehr; reale native Bedienung bleibt nachzuprüfen.

## Geprüfter Teilumfang

115 produktive Readerprüfungen erfassen zwölf gewöhnliche Formtypen, den Guard,
identische Quellen-/Workerobjekte, unveränderte Parserzähler, sofortige Rückkehr,
Readerposition und Inhaltshöhe sowie Editorposition nach Settings. Weiterhin
geprüft: Resize, Schriftwechsel, Retargeting, Editieren, literal CRLF-Quellen und
modaler Kontextreader. Reader-/Titelgeometrie, native Feldaktionen, Schutzdialog,
Interaktion und erster Start bestehen in den gezielten Läufen.

177 Quell-/Stil-, 115 Reader- und 621 native Mac-Assertions bestehen auch unter
haltendem ASan/UBSan: 39 eigene C-Dateien und 55 aktuelle Quell-/Include-Hashes.
Externe UI-Abhängigkeiten nicht instrumentiert; keine Leak-/ThreadSanitizer-
oder tatsächliche VoiceOver-Abnahme. Drei finale Softwarebilder zeigen die
stabile Inhaltsposition vor/Aktionen/nach Rückkehr. Die ersten Bilder mit
laufender Überblendung beziehungsweise zusätzlicher H1 sind Vorfassungen,
keine Abschlussbilder. Kein allgemeiner FPS-/GPU-/Trackpadnachweis daraus.

Originale: `build/reader-overlay-verification/`, insbesondere `probe.log`,
`title-corrected.log`, `image-final.log` und `images-final/`; Speicherprüfung unter
`build/reader-overlay-sanitizers/`. Finale Bilder und Apple-Zugriffsdateien liegen
im verlinkten UI-Bericht. Geprüft: macOS 14.6.1/23G93, Intel, Softwareausgabe
1336×840 mit Retina-Backing; Reader-Prüfer zusätzlich 750×900/150%-Schrift.
Weitere Zielplattformen, echte Geräte/IME/Screenreader und 1.0 bleiben offen.

## Grundlage und verbleibender Formularbefund

Apple [Modality](https://developer.apple.com/design/human-interface-guidelines/modality),
[Loading](https://developer.apple.com/design/human-interface-guidelines/loading) und
[Scroll views](https://developer.apple.com/design/human-interface-guidelines/scroll-views)
am 11. Oktober erneut über titelgeprüfte DocC-Daten gelesen. Apple benennt eine
begrenzte modale Aufgabe und den vorigen Kontext. Die konkreten Cache-/Scrollslot-
und C-Kontextregeln sind eigene Übertragung.

SBUI-080 ist separat offen: F1 aus einem begonnenen Notizformular ersetzt dieses
durch Hilfe; Escape kehrt zur Hauptansicht statt zur Formularaufgabe zurück.
Erneutes Anlegen leert den eingegebenen Titel. Die Ereignisprobe bestätigt
Form 2→6→0→2 und den Verlust der noch nicht gespeicherten Eingabe, keinen Verlust
einer vorhandenen Datei. Die künftige Korrektur muss die ursprüngliche Aufgabe,
Fokus/Auswahl und Eingaben erhalten. Dieser Schritt schließt den Befund nicht.
