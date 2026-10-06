# Erster Start und leerer Arbeitsordner

Stand: 6. Oktober 2026. Recherche vor dem Entwurf; implementiert in Entwicklungsversion 0.7.0.

## Grundlage

[Apple HIG: Onboarding](https://developer.apple.com/design/human-interface-guidelines/onboarding),
am 6. Oktober 2026 aus den öffentlichen Dokumentationsdaten gelesen: schneller,
optionaler Einstieg; durch Aufgaben lernen, Hinweise im Kontext anbieten und
nicht notwendige Anpassungen verschieben. Ein eigener Tutorial- oder Lizenzdialog
ist dafür keine Voraussetzung. Die Übertragung auf den leeren Arbeitsordner ist
eine eigene Gestaltungsentscheidung, keine von Apple vorgeschriebene Ansicht.

## Entwurf und Verhalten

Ohne geöffnetes Projekt zeigt die App eine zentrale schwebende Einstiegskarte
im Lumen-Raum. Es gibt keine Dokumentwerkzeuge, leere Suche oder Kameraaktionen.
Ein kurzer Hinweis erklärt das Projektgedächtnis; der aktuelle Arbeitsordner
zeigt, wo ein neues Projekt angelegt wird. Vier direkte Wege führen zum bestehenden
Projektformular, zur Ordnerwahl, zur Wiederherstellung und zur Tastaturhilfe.
Darstellung bleibt erreichbar. Ein Tutorial muss nicht abgeschlossen werden.

Die Ansicht entsteht aus dem tatsächlichen Projektzustand; ein zusätzlicher
persistenter Erststart-Schalter ist nicht nötig. Öffnen oder Erstellen eines
Projekts führt zur normalen Arbeitsansicht. Abbruch kehrt zur Einstiegskarte
und ihrem vorherigen Fokus zurück. Ein leerer anderer Arbeitsordner zeigt dieselben
Wege. Bereits vorhandene Projekte bleiben im normalen Arbeitsablauf.

Die Überschrift bleibt fest; der Inhalt kann bei kleinem Fenster oder großer
Schrift scrollen. Tab erreicht alle Aktionen und scrollt den Fokus ins Bild.
Native Semantik enthält Überschrift, Erklärung, Arbeitsordner und beschriftete
Aktionen. Fehler werden in der Karte angezeigt. Die Ansicht erzeugt kein
Beispielprojekt und verändert keine vorhandenen Projektdateien.

## Abnahme

Ersten Start mit privaten Daten, bestehendem Projekt und anderem leeren Ordner
prüfen. Anlegen, Öffnen, Wiederherstellen, Hilfe und Abbruch per Tastatur;
kleines Fenster, 200 Prozent Schrift, beide Darstellungen und Kontrast betrachten.
Native Provider-/Screenreaderabnahme ist ein eigener Nachweis. Die Prüfung des
Erstellungswegs muss bestehende Arbeitsabläufe und Bestandsschutz erhalten.


## Lokaler Nachweis

Der gesamte lokale Release-Lauf besteht mit 17 Tests (177,06 Sekunden). Die
gezielte Prüfung umfasst die leere Ansicht, Tastaturwege und Abbruch, 200 Prozent
Schrift in der tatsächlichen Mindestgröße, Projektwechsel und Rückkehr zum leeren
Ordner. Dunkle Lumen- und helle Kontrastansicht wurden tatsächlich betrachtet.
Die abschließende Mausrad-/Sanitizer- und Plattformabnahme folgt gesondert.


Abschließende lokale Nachprüfung: erster Start mit 40 Aussagen einschließlich
Mausrad innerhalb/außerhalb der Karte besteht (5,56 Sekunden). ASan/UBSan bestehen
für ersten Start, Systemdarstellung und native Zugänglichkeit (42,09 Sekunden;
40/51/121 Aussagen). Der volle lokale Release-Lauf enthält 17 bestandene Tests.
Die Plattform- und entpackte Paketabnahme zu 0.7.0 steht noch aus.
