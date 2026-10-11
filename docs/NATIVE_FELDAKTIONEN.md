# Native Auswahl und Ersetzung in Eingabefeldern

Stand: 11. Oktober 2026, Umsetzung ab 0.9.50.

Suche und Formulartexte bieten neben dem vollständigen Setzen des Werts die
nativen Aktionen `SetTextSelection` und `ReplaceSelectedText`. Der vorhandene
Notizeditor behält seinen eigenen Edit-/Undo-Pfad. [Geometrie](NATIVE_TEXTGEOMETRIE.md)
und [native Textläufe](NATIVE_TEXTLAEUFE.md) liefern die Quellzuordnung.

## Auswahl und Inhalt

Die Desktop-Auswahl zählt Unicode-Skalarwerte; der Provider übersetzt native
Laufpositionen auf diese Koordinaten. Eine ausgedehnte Auswahl umfasst ganze
Grapheme, auch bei umgekehrter Richtung. Eine kollabierte Position innerhalb
eines Graphems geht an dessen Anfang. Auswahlgrenzen außerhalb des aktuellen
Texts werden abgewiesen.

Jedes Feld hält seine letzte Auswahl mit exakten Quellenbytes und Projekt-/
Dialogkontext. Native Fokusrückkehr stellt sie im richtigen Nuklear-Widget
wieder her. Nuklear verwendet ein gemeinsames kurzlebiges Editobjekt; eine
ausstehende Auswahl wird deshalb vor dem passenden Widget angewendet und nach
der tatsächlichen aktiven Eingabe übernommen. Andere Felder verbrauchen sie
nicht. Kontextwechsel verwerfen den alten Auswahlzustand.

Ersetzungen werden vollständig vorbereitet, bevor die Quelle geändert wird.
Ungültiges UTF-8, Zeilenumbrüche und Überschreitung der Feldkapazität erhalten
Quelle und Auswahl. Eine leere Ersetzung löscht die Auswahl; eine leere Auswahl
fügt an der Einfügestelle ein. Der Cursor landet graphemgebunden hinter dem
eingefügten Quellabschnitt. Auswahl und Ersetzung können in derselben nativen
Warteschlange nacheinander verarbeitet werden.

Native Rückmeldungen benötigen den noch gültigen Kontext und die aktuelle
Generation. Hat sich der Feldtext gegenüber der Momentaufnahme geändert,
wird eine darauf bezogene Auswahl/Ersetzung abgewiesen. Erfolgreiche Änderungen
der manuellen Dateikennung verhindern erneutes automatisches Ableiten;
Änderungen eines Sicherungspfads verwerfen die dazugehörige geprüfte Vorschau.
Reine Auswahl verändert diese Zustände nicht.

## macOS-Anschluss

Der festgelegte `accesskit_macos 0.27.1` bietet `setAccessibilitySelectedTextRange:`
und den Lesewert `accessibilitySelectedText`, aber keinen Setter
`setAccessibilitySelectedText:`. Die eigene C-Anbindung ergänzt diesen am
Providerknoten. Das betrifft auch den bestehenden Notizeditor.

Die Anfrage wird über Provider-Autor-ID und natives Fenster dem Besitzer
zugeordnet. Unter den vorhandenen Sperren prüft die Anbindung das noch lebende,
editierbare Element und verwendet die normale begrenzte Ereigniswarteschlange.
Schreibgeschützte Elemente bieten den Setter nicht als erlaubte Aktion an und
weisen auch einen direkten Aufruf zurück. Eingebettete NUL-Zeichen werden über
die vollständige native UTF-8-Bytelänge erkannt und nicht als verkürzter Wert
übernommen. Die Besitzerzuordnung wird vor Adapter-/Mutex-Freigabe entfernt.

Primärquelle: die lokal gelesene, festgelegte
[Providerquelle 0.27.1](https://docs.rs/crate/accesskit_macos/0.27.1/source/src/node.rs),
Abschnitte `set_selected_text_range`, `set_value` und erlaubte Selektoren.
Die lokale Herkunft und der Quellenhash stehen in
`build/field-actions-verification/provider-source.json`.

## Prüfungen und Grenzen

Der produktive Feldprüfer verwendet die echte Queue, Tick, Widget, Rasterung
und Veröffentlichung für alle sieben Feldtypen. Er prüft Graphemgrenzen,
Richtung, Löschen/Einfügen, leere Texte, exakte/überschrittene Kapazität,
Fokusrückkehr, Änderungen zwischen Rückmeldung und Verarbeitung sowie geschlossene
Kontexte. Der Mac-Providerprüfer ergänzt native Auswahl und Ersetzung im Titel
und Notizeditor, NUL-Abweisung, schreibgeschützte Dokumente und getrennte
Warteschlangen gleich benannter Felder in zwei Fenstern.

Ausgeführte Ergebnisse und Instrumentierungsumfang stehen in [STATUS](STATUS.md).
Neue Windows-/Linux-Providerabnahme, echte IME, VoiceOver/NVDA/Orca und
Geräte-/Langzeitabnahmen bleiben offen. Der neue Anschluss ersetzt diese
Abnahmen und die verbleibende native Leseblock-Geometrie nicht.
