# Anforderungen und Abnahme

Stand: 6. Oktober 2026. Quelle: [Projektplan](../../../docs/PROJEKTPLAN.md).

Für dieses Produkt gehören Projektauftrag, datierter Stand, Entscheidungen,
offene Fragen und Quellen zum kleinen gemeinsamen Kern.

Die App-Abnahme folgt einem tatsächlichen Arbeitsablauf: Projekt anlegen,
Wissensnotiz erstellen, Unicode-Text bearbeiten, speichern, lesen, suchen und
wieder öffnen. Quellen werden schreibgeschützt betrachtet und Kerninformationen
kopiert. Wechsel dürfen ungespeicherte Änderungen nicht verlieren.
Das [UI-Testprogramm](../../../app/self_test.c) bedient dieselben Komponenten
mit Maus-, Tastatur- und Zwischenablageereignissen.

Bausteintests, Bedienablauf und entpacktes Paket belegen verschiedene Ebenen.
Ein erfolgreicher Generator- oder Renderer-Test ersetzt keine Desktop-Prüfung.
Jeder Nachweis benötigt Commit, Plattform und tatsächliches Ergebnis.
Beleg: [Plattformnachweise](../../../docs/PLATTFORMEN.md).

Die UI-Recherche wurde zuerst dokumentiert. Apple-Orientierung bedeutet ruhige
Hierarchie, klare Typografie, Platz für Inhalte und konsistente Rückmeldungen.
Die App übernimmt die Bedienkonventionen der jeweiligen Plattform.
Beleg: [UI-Recherche](../../../docs/UI_RECHERCHE.md).
