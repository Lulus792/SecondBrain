# Lizenzen in der Anwendung

Stand: 7. Oktober 2026, Entwicklungsstand 0.9.3.

Unter **Darstellung → Über SecondBrain → Lizenzen** stehen die mitgelieferten
Originaltexte des eigenen Codes, der UI-Komponenten und der Schriften. Der
Lesebereich zeigt sie unverändert als Text; Markdown-Zeichen werden nicht
interpretiert. **Lizenztext kopieren** übernimmt den vollständigen Originaltext.
Auf Windows kann die Zwischenablage LF in CRLF umsetzen.

**Zurück zur Übersicht** führt zur Auswahl; **Zurück zu Über SecondBrain**
führt zur Versionskarte. Das feste Schließen-Icon und Escape beenden die
Ansicht. Tab und Umschalt+Tab erreichen alle Einträge, auch bei großer Schrift.
Bild auf/ab und Mausrad bewegen den Text mit demselben begrenzten weichen
Scrollen wie in anderen Lesebereichen. Offene Notizentwürfe bleiben erhalten.

Die Anwendung liest ausschließlich bekannte Lizenzdateien aus ihren eigenen
Ressourcen. Fehlende oder ungültige Dateien zeigen einen Fehler in der Auswahl;
sie öffnen keinen leeren Lesebereich und ersetzen keine Projektdaten. Python
und ein externer Editor sind nicht erforderlich.

## Gestaltung und Quellen

Die bestehenden Modal- und Scrollkomponenten werden wiederverwendet. Die erneut
am 7. Oktober gelesenen Apple-Kapitel
[Modality](https://developer.apple.com/design/human-interface-guidelines/modality)
und [Scroll views](https://developer.apple.com/design/human-interface-guidelines/scroll-views)
stützen eindeutige Aufgabenüberschriften, einen klaren Rückweg und das Beibehalten
der üblichen Scrollbedienung. Die Originalinhalte wurden über Apples zugehörige
Dokumentations-JSON gelesen. Eigene Übertragung: eine Übersicht und ein
Lesebereich innerhalb derselben Karte; die Kopfzeile bleibt fest. Nur der
aktive Inhaltsbereich scrollt. Das ist eine eigene C-Umsetzung.

## Umfang

17 Einträge umfassen MIT für SecondBrain, SDL3, Nuklear, die Noto-Schriften,
SDL_ttf, FreeType einschließlich BDF/PCF/zlib-Anteilen, HarfBuzz einschließlich
Microsoft USE sowie AccessKit einschließlich Chromium-Anteilen und Autoren.
[Herkunft, Versionen und Anpassungen](../third_party/README.md).

Die sichtbare Übersicht ersetzt keine vollständige Prüfung aller transitiven
UI-Abhängigkeiten oder die noch offene Support- und Sicherheitsmeldestruktur.
Plattformprüfungen und ihre Grenzen stehen im [Umsetzungsstand](STATUS.md).


## Unicode-Daten ab 0.9.8

Die Übersicht umfasst nun 18 Texte, einschließlich Unicode License V3 für die
festgelegten Eigenschaftsdaten und abgeleiteten Graphemtabellen. Die eigene
Segmentierung bleibt C-Code unter der Projektlizenz. Der Unicode-Lizenztext
liegt in den lokalen UI-Ressourcen und jedem Paket; der Pakettest verlangt ihn.
Die Ressourcennachprüfung mit 18 Originalen besteht lokal. [Herkunft und Hashes](../third_party/unicode/README.md).


## Emoji-Schrift ab 0.9.9

21 Originaltexte sind angebunden. WHATWG und Noto Math ergänzen Daten-/Fontlizenzen. Noto Emoji ergänzt die SIL-OFL-Ressource;
Quelle und unveränderte Schriftdatei sind im [Emoji-Vertrag](EMOJI.md) festgelegt.
Die lokale Ressourcennachprüfung besteht mit 58 Assertions. Die neue Paketprüfung
verlangt Schrift und Lizenz; ihren tatsächlichen Abschluss dokumentiert STATUS.md.

## Transitive Originale ab 0.9.20

23 Einträge enthalten nun zusätzlich die festgelegten Cargo-Unterabhängigkeiten
von AccessKit und Quellenhinweise aus SDL3/HarfBuzz. Sammlungen behalten ihre
Originaltexte und Attributionsangaben; die App liest sie als Literaltext.
[Inventur, Wiederholung und offene Laufzeitanteile](LIZENZ_INVENTUR.md).
