# CommonMark-Hervorhebungsfälle

Unveränderte Fälle des Abschnitts „Emphasis and strong emphasis“ aus
[CommonMark 0.31.2](https://spec.commonmark.org/0.31.2/spec.json),
John MacFarlane, 28. Januar 2024. Auswahl aus dem Originaldatensatz: 132 Fälle.
Die ursprünglichen Beispielnummern bleiben erhalten. Datenlizenz:
[CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/).
Originaldatensatz SHA-256: `d431b29d97b6f73e69d547109cf5081578fac931e72afe95639ebe766c1b2a20`.

Die Fälle prüfen den eigenen C-Leser. Die Desktop-Leseansicht zeigt rohe HTML-
Tags als Literaltext und führt sie nicht aus; bei den drei reinen HTML-
Interaktionsfällen ist dieser Anzeigevertrag ausdrücklich die erwartete Ausgabe.
Andere Fälle behalten die originale Text-/Hervorhebungsstruktur. Dies ist
keine vollständige CommonMark-/HTML-Konformitätssuite.


Entity-Auswahl: 15 unveränderte Beispiele 25–41; Referenzlinkfall 33 und
Listenfall 38 bleiben bis zur jeweiligen Container-/Referenzimplementierung
offen. Fall 31 verwendet die dokumentierte wörtliche HTML-Anzeige. Die
ursprünglichen Beispiele und ihre Lizenz bleiben erhalten.

Autolinks: alle 19 unveränderten Beispiele 594–612 aus demselben Original.
Text, Stile und tatsächlich gelesene Ziele werden mit der HTML-Ausgabe
verglichen. Keine angepasste Erwartung für diese Auswahl. Ergänzende eigene
E-Mail-Grenzfälle verwenden einen unabhängigen Regex-Vergleich.


Referenz-Auswahl: 81 unveränderte Beispiele, einschließlich Entity-Ziel 33,
Definitionsfälle 192–217 außer Containerfall 214, Inline-Referenzen 527–571
und Bilder 582–591. Containerfall 218 bleibt ebenfalls offen. Zwei Roh-HTML-
Fälle (201/536) folgen dem Literalvertrag, zwei Bildfälle (585/589) dem bereits
bestehenden aufbereiteten Stil des Alternativtexts. Originaldaten und Lizenz
bleiben erhalten; Text/Stile/Ziele werden mit diesen ausdrücklichen Produkt-
verträgen verglichen. Dies behauptet keine vollständige CommonMark-Abnahme.
