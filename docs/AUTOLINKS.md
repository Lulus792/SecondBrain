# Adressen in der Leseansicht

Stand: 7. Oktober 2026, Entwicklungsschritt 0.9.15.

Der gemeinsame eigene C-Leser erkennt URI- und E-Mail-Autolinks in
Winkelklammern. Titel, Absätze, Tabellen und native Textwerte verwenden dieselbe
Projektion. `<name@example.com>` zeigt die Adresse ohne Winkelklammern;
das Linkziel ist `mailto:name@example.com`. Die Datei bleibt unverändert.
Normale Adressen ohne Winkelklammern bleiben Text. Code und maskierte
Winkelklammern erzeugen keine Aktionen. E-Mail-Adressen dürfen entsprechend
der CommonMark-Regel nur ASCII enthalten; jedes Domainsegment umfasst höchstens
63 Zeichen und beginnt/endet alphanumerisch. Dies prüft die Markdown-Grammatik,
nicht die Zustellbarkeit einer Adresse.

## Anzeigen und Öffnen

Autolinks behalten Sonderzeichen wörtlich. Der Leser dekodiert darin weder
Entities noch Maskierungen. Sie aktivieren keine Hervorhebungen im Inneren.
Ein Autolink verhindert einen umgebenden weiteren Link; ein Bild behält seine
eigenen Alternativtext-Regeln. URI-Autolinks schließen auch DEL aus.

Die vorhandenen Linkknöpfe sind mit Tab und Enter erreichbar. HTTP/HTTPS öffnen
über die OS-Anbindung; `mailto:` fordert die konfigurierte E-Mail-App an.
Bei einem automatisch erzeugten E-Mail-Ziel kodiert die App Sonderzeichen der
Empfängeradresse vor der OS-Übergabe einmal. `?`, `#`, `&`, `%` und andere
URI-Zeichen bleiben so Bestandteil der Adresse, ohne zusätzliche Mailfelder
zu bilden. Ein ausdrücklich geschriebenes `mailto:`-URI behält seine eigene
URI-Notation. Die App versendet keine Nachricht. Andere URI-Schemata bekommen
durch diese Änderung keine allgemeine Ausführungsbefugnis; der bisherige
Quellenpfad meldet nicht unterstützte Ziele. Adressen erzeugen keine lokalen
Sternkartenbeziehungen.

Originalregeln: [CommonMark 0.31.2, Autolinks](https://spec.commonmark.org/0.31.2/#autolinks),
[IETF RFC 6068, Syntax und Kodierung](https://www.rfc-editor.org/rfc/rfc6068.html#section-2)
und [SDL_OpenURL](https://wiki.libsdl.org/SDL3/SDL_OpenURL), gelesen am 7. Oktober 2026.
Der URL-Start bestätigt die Übergabe an das Betriebssystem; eine tatsächlich
bediente E-Mail-App bleibt ein eigener Plattformtest.

## Nachweise und Grenzen

19 unveränderte CommonMark-Fälle vergleichen echten C-Klartext, Stile und Ziele.
1.000 zusätzliche deterministische Adressfälle bestehen gegen einen separaten
Regex-Vergleich. Eigene Fälle prüfen Maskierung, Code, verschachtelte Links,
63/64 Zeichen im Domainsegment, DEL und exakte Zielpuffer. Die App-Prüfung
vergleicht Absatz-/Zellwerte und verwendet Enter, um zwei tatsächliche
Linkaktionen an einen aufgezeichneten URL-Adapter zu übergeben. Sie bestätigt
kodierte Empfänger und unveränderte Quelle, ohne externe Programme zu starten.
Das Raster wurde betrachtet. Aktuelle vollständige Ergebnisse stehen in
[STATUS](STATUS.md); dies ist keine menschliche Screenreader-/Mailclient-Abnahme.

Referenzlinks benötigen zusätzlich eine dokumentweite Definitionsliste und
Unicode-Normalisierung ihrer Namen. Die [nächste Parserarbeit](REFERENZLINKS_RECHERCHE.md)
ist recherchiert, aber noch nicht implementiert. Vollständige Container,
Bidi/Textgeometrie/IME und übrige Release-Aufgaben bleiben offen.
