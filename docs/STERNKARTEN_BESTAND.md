# Letzter gültiger Stand der Sternkarte

Stand: 7. Oktober 2026, ab 0.9.19. Die Desktop-Sternkarte übernimmt einen
Neuaufbau erst, wenn Graph, Beschriftungen, Dokumentpfade, Kennungen und
Darstellungspunkte vollständig vorliegen. Bei Lese-, Parser-, Größen- oder
Speicherfehlern bleibt der vorherige Stand desselben Projekts erhalten.
Der Rückmeldebereich meldet den Fehler und bezeichnet die Karte als letzten
gültigen Stand. Die Dokumentliste bleibt die aktuelle Inventur.

## Zuordnung und Bedienung

Eigener C-Code hält eine besessene Kopie von `SBNote`-Metadaten neben dem Graph.
Sterne, Suche-/Bereichsfilter, Beschriftungen, Auswahl, Kameraziele, Maus und
Tastatur benutzen diese Kopie. Native Aktionen erhalten eine eigene Kennung;
sie wird anhand des Dokumentpfads bei einem erfolgreichen Neuaufbau erhalten.
Eine umsortierte oder kleinere aktuelle Liste kann damit keine alten Sterne
neu beschriften oder andere Dokumente öffnen.

Kennungen gelten für die laufende Anwendung, sind monoton vergeben und werden
bei Projektgrenzen nicht wiederverwendet. Ein umbenannter Pfad erhält eine neue
Kennung. Das ist keine Migration des Dateiformats und keine neue persistente
UUID. Native Kontext-/Generationsprüfungen gelten weiterhin; überholte Aktionen
werden nicht auf einen anderen Kontext angewandt.

Eine Aktion öffnet den gespeicherten relativen Pfad im aktuellen Projekt. Sie
liest die jetzigen Bytes, keine gespeicherte historische Dokumentfassung.
Fehlt die Datei oder ist sie ungültig, gilt der vorhandene Fehler-/Entwurfsschutz.
Die Karte wird bei der nächsten vorgesehenen Aktualisierung erneut aufgebaut;
es gibt noch keinen automatischen Dateiwächter. Ein Projekt- oder
Arbeitsordnerwechsel verwirft den vorherigen Graph vor dem Neuaufbau.

## Nachweise

`tests/test_graph_snapshot.c` provoziert eine extern ungültig geschriebene Datei
und gleichzeitig verkleinerte/umgeordnete Inventur. Die letzte vollständige Karte
bleibt mit ihren Verweisen, Titeln und Kennungen erhalten; echte native Aktionen
und Tastaturwege benutzen die alten richtigen Pfade. Ungültige Ziele verändern
keinen offenen Text. Wiederherstellung, ein neues Dokument, umsortierte Inventur,
erhaltene Kennungen und eine fehlerhafte neue Projektkarte sind geprüft.

Aktuelle Gesamt-, Sanitizer-, Paket- und Plattformnachweise: [STATUS](STATUS.md).
Der Kern erhielt seinen Graph schon vorher; dieser Vertrag beschreibt die neue
zusammenhängende Desktop-Sicht. Weitere [Release-Abnahmen](RELEASE.md) bleiben offen.
