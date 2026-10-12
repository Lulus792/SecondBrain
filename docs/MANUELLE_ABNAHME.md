# Abnahme auf einem echten Rechner

Stand: 12. Oktober 2026. Dieser Ablauf ist vorbereitet, noch nicht als menschliche
Abnahme ausgeführt. Er ergänzt die automatisierten Nachweise und die
[Release-Kriterien](RELEASE_ABNAHME.md). Jedes zugesagte System und jede
Architektur benötigt einen eigenen Bericht. Ein bestandener Durchlauf gibt 1.0
nicht automatisch frei.

## Umgebung und Ausgangspunkt

Ein entpacktes Paket verwenden, dessen Herkunft und Prüfsumme festgehalten sind.
Die [Systemziele](PLATTFORM_VORSCHLAG.md) sind vorgeschlagen, noch nicht zugesagt.
Für X11 und Wayland getrennt prüfen. Nicht aus einem Entwicklerbuild auf die
Funktionsfähigkeit des veröffentlichten Pakets schließen.

Für den Durchlauf einen neuen Arbeitsordner mit erfundenen Inhalten anlegen.
Die vorhandenen Wissensbasen nicht verwenden. Testdateien während der Prüfung
aufbewahren. Keine Schutzmaßnahmen des Betriebssystems für die Abnahme umgehen;
eine Startblockade mitsamt Hinweis dokumentieren.

Zu Beginn diese Angaben in einen neuen Ergebnisbericht kopieren:

| Angabe | Tatsächlicher Wert |
| --- | --- |
| Datum und prüfende Person | |
| Paketquelle, Archivname und SHA256 | |
| Version/Quellstand aus „Buildangaben kopieren“ | |
| OS-Version, Architektur, Desktop/Fenstersystem | |
| Gerät, GPU/Treiber, Maus oder Trackpad | |
| Monitore, Auflösung und Skalierung je Monitor | |
| Screenreader samt Version und Eingabemethode | |
| Systemvorgaben und App-Darstellung | |

Für jeden folgenden Fall **bestanden**, **fehlgeschlagen** oder **nicht geprüft**
eintragen, dazu die Beobachtung. Ein nicht verfügbarer Monitor, Screenreader oder
Eingabemodus ist kein bestandener Fall. Aufnahmen nur mit Testinhalten erstellen.

## Projekt, Tastatur und Navigation

Command gilt auf macOS, Control auf Windows/Linux. Die aktuellen Wege stehen in
der [Bedienung](../README.md#bedienung) und im [Tastaturvertrag](UI_TASTATUR.md).

| Fall | Handlung | Erwartetes Ergebnis |
| --- | --- | --- |
| M01 Start und Erstnutzung | Paket ohne Entwicklungswerkzeuge starten. Über „Neues Projekt“ ein Testprojekt anlegen. App regulär beenden und neu öffnen. | Erstnutzung verständlich; Projekt und Arbeitsordner werden wiedergefunden. Auf dem frischen Rechner keine ungenannte Laufzeitinstallation nötig. |
| M02 Sofortige Pfeilwahl | Im Testprojekt mindestens drei Notizen in verschiedenen Bereichen anlegen. Neustarten und ohne Mausklick Pfeile drücken. Danach schnell mehrfach die Richtung wechseln. | Auswahl reagiert direkt, ohne Enter. Beschriftung, gewählte Notiz und Übergang passen zusammen; keine Eingaben gehen durch eine laufende Animation verloren. |
| M03 Archiv verlassen | Archivnotiz öffnen; die Bereichswahl ausschließlich mit Tastatur öffnen und zurück zu allen Dokumenten wechseln. | Bereichswahl bleibt erreichbar. Andere Notizen erscheinen wieder; kein Sackgassen-Zustand. |
| M04 Aktionen und Fokus | Mit Umschalt+F10 Aktionen öffnen, mit Auf/Ab wechseln, eine harmlose Aktion aktivieren. Erneut öffnen und mit Escape schließen. Tab, Umschalt+Tab und F6 durch die Oberfläche verwenden. | Sichtbarer Fokus, passende Aktion, keine Tastaturfalle. Escape führt an den sinnvollen Ausgangspunkt zurück. |
| M05 Suche und Rückkehr | Liste schließen, suchen, Suchfeld mit integriertem X leeren und Suche verlassen. Dasselbe mit vorher offener Liste wiederholen. | Die nur durch Suche geöffnete Liste schließt; eine zuvor offene Liste bleibt offen. Suchfeld, X und Fokus sitzen sauber. |
| M06 Formulare und Hilfe | Neues Projekt und neue Notiz mit langen Titeln vorbereiten. Mit F1 Hilfe öffnen und zurückkehren; schließlich abbrechen. Bei großer Schrift wiederholen. | Beschriftungen, Eingaben und Schließen bleiben erreichbar, ohne Überdeckung. Hilfe erhält Entwurf und Auswahl; Abbrechen erzeugt kein halbfertiges Projekt. |
| M07 Große Leseansicht | Längere Notiz mit Überschriften, Liste, Tabelle und Code über „Groß lesen“ öffnen; scrollen, Abschnitt wechseln und zurückkehren. | Lesefläche wird größer; Text bleibt lesbar und unverändert. Rückweg und Aktionen sind erreichbar. |

## Bewegung, Layout und Texteingabe

| Fall | Handlung | Erwartetes Ergebnis |
| --- | --- | --- |
| M08 Kamera und Übergang | Mit Pfeilen zwischen räumlich getrennten Sternen wechseln, die Kamera ziehen und zoomen; während des Übergangs erneut wechseln. | Bewegung vermittelt Orientierung, folgt Eingaben und bleibt unterbrechbar. Kein spätes Loslaufen, kein falscher alter Inhalt oder Zuschnitt. Subjektive störende Verzögerungen ausdrücklich beschreiben. |
| M09 Scrollgrenzen | Lange Notiz mit echtem Trackpad und Maus bis oben/unten scrollen; weiterrollen, Richtung wechseln, Scrollgriff ziehen. | Fließender Verlauf und stabiler Rand. Wiederholtes Scrollen am Ende schleudert den Inhalt nicht zurück. Griff folgt der Hand. Maus und Trackpad getrennt protokollieren. |
| M10 Fenster und Dichte | Fenster klein/groß einstellen, maximieren; auf einen anderen Monitor mit anderer Skalierung verschieben. Menüs, Tooltip, Suche und Formulare öffnen. | Keine abgeschnittenen Texte oder Icons; passende Innenabstände. Fokus, Textcursor, Trefferflächen und Inhalt bleiben deckungsgleich. Ohne zweiten Monitor bleibt der Dichtewechsel ungeprüft. |
| M11 Editor | Text markieren, wortweise bewegen, löschen, Undo/Redo und speichern. `Äpfel`, `é`, `👩🏽‍💻` und eine gemischte RTL-Zeile verwenden. Mit Tab das Feld verlassen. | Cursor und Auswahl entsprechen der sichtbaren Position. Zusammengesetzte Zeichen werden als Einheit behandelt; gespeicherter Text bleibt erhalten. |
| M12 Native IME | Eine tatsächlich installierte Eingabemethode verwenden: Komposition beginnen, Kandidaten wählen, abbrechen und während einer Komposition den Fokus wechseln. | Kandidatenfenster beim Cursor; keine doppelten, verlorenen oder nachträglich im falschen Feld eingefügten Zeichen. Eingabemethode und konkrete Zeichenfolge festhalten. |
| M13 Systemdarstellung | Große Schrift, reduzierte Bewegung/Transparenz und helle Darstellung prüfen. Unterstützte Systemvorgaben sowie unter Windows ein eigenes Kontrastthema verwenden. | Lesbarkeit und Bedienwege bleiben erhalten; reduzierte Bewegung wird berücksichtigt. System-/App-Auswahl und tatsächlich sichtbare Reaktion getrennt notieren. |

## Menschliche assistive Bedienung

VoiceOver auf macOS, NVDA auf Windows und Orca unter Linux tatsächlich benutzen.
Eine API-Abfrage oder ein synthetisches Providerereignis genügt hier nicht.
Den passenden Screenreader zunächst ohne Maus verwenden; nötige Mausauswege
als Befund dokumentieren. [Native Anbindung und Grenzen](BARRIEREFREIHEIT_PLAN.md).

| Fall | Handlung | Erwartetes Ergebnis |
| --- | --- | --- |
| M14 Namen, Rollen und Auswahl | Projektwahl, Sternkarte, Suche und Aktionen erkunden; zwischen Notizen wechseln und Menü schließen. | Verständliche Namen/Rollen/Zustände, richtige gewählte Notiz, nachvollziehbarer Fokus; kein unsichtbarer Hintergrund als aktives Modalziel. |
| M15 Lesen und Bearbeiten | Große Leseansicht öffnen; Überschriften, Liste, Tabelle und Links erkunden. Editor öffnen, Auswahl bewegen, ändern und speichern. | Inhalt und relevante Struktur zugänglich; Auswahl/Cursor und Ansagen passen zum Text. Tabellenbedienung mit tatsächlichen Ansagen protokollieren. |
| M16 Formulare und native Dialoge | Projekt/Notiz anlegen und native Ordnerwahl öffnen. Dialog abbrechen; Fehler mit einem ungültigen Testpfad auslösen. | Felder, Fehler und Aktionen verständlich; Dialog bedienbar und Fokus kehrt zurück. Kein stummes Scheitern oder verlorener Entwurf. |

## Speicherung, Sicherung, Update und Alltag

| Fall | Handlung | Erwartetes Ergebnis |
| --- | --- | --- |
| M17 Entwurf und Konflikt | Ungespeicherten Text ändern und Notiz/Projekt wechseln; Abbrechen wählen. Danach die gespeicherte Testnotiz außerhalb der App ändern und den Entwurf speichern. | Abbrechen erhält Entwurf. Konflikt wird gemeldet; fremde Inhalte werden nicht still überschrieben. [Datenvertrag](DATENVERTRAG.md). |
| M18 Sicherung | Testprojekt sichern, Vorschau prüfen und unter freier Kennung wiederherstellen. Einen vorhandenen Zielnamen versuchen; laufenden Vorgang abbrechen. | Inhalte/Metadaten erhalten; bestehendes Ziel geschützt; Abbruch nachvollziehbar. [Sicherungsvertrag](SICHERUNG.md). |
| M19 Paketwechsel | Ausgangspaket und Einstellungen sichern. Altes Paket → Prüfpaket → altes Paket mit demselben Testarbeitsordner ausführen; Dateipfade und SHA256 vor/nach jedem Schritt vergleichen. | Gespeicherter Ort, Notiz, Einstellungen und Originalbytes erhalten. Unterschiedliche Paketidentitäten und tatsächliche Startweise festhalten. [Updateablauf](UPDATE_NACHWEIS.md). |
| M20 Alltag | Repräsentative Kopie einer größeren Wissensbasis über eine längere Sitzung bedienen: suchen, wechseln, bearbeiten, sichern; Fenster/Monitore wechseln. | Keine wachsende Verzögerung, verlorenen Eingaben oder Abstürze. Dauer, Datei-/Notizzahl, Größe, Gerät und beobachtete Leistung festhalten; kein Zeit-/Mengenschwellenwert gilt dadurch als Produktzusage. |

Physische Stromausfall-/Persistenzversuche und weitere Dateisysteme benötigen
einen getrennten kontrollierten Prüfaufbau. Dieser Bedienablauf führt sie nicht
aus und schließt sie nicht ab. [Bisherige Volumenprüfungen](VOLLER_SPEICHER.md).

## Ergebnis und Rückmeldung

Im Bericht jeden Fall mit Ergebnis und Nachweis benennen. Bei einem Fehler die
konkrete Eingabefolge, erwartetes und beobachtetes Verhalten sowie die Umgebung
festhalten. Nach einer Korrektur den betroffenen Ablauf erneut prüfen; den alten
Befund erhalten. Private Daten vor Weitergabe entfernen. Gewöhnliche Fehler über
[Support](../SUPPORT.md), vermutete Sicherheitslücken über den
[vertraulichen Kanal](../SECURITY.md) melden.

Ein vollständiger Bericht nennt ausdrücklich die nicht geprüften Fälle und
stützt nur die tatsächlich verwendete Umgebung. Signierung, Notarisierung,
Lizenzabschluss und ausdrückliche 1.0-Freigabe bleiben separate Voraussetzungen.
