# Bibliothek App und Sprache in Physim

Die Physim-Bibliothek liefert den fachlichen Kern. Die App verbindet Entwicklung,
Simulation und Auswertung. C und die eigene Physim-Sprache sollen denselben
fachlichen Kern für Experimente und Analysen verwenden.

## Herkunft und Gültigkeit

Die Trennung von Bibliothek und App ist ein verbindliches Ziel aus dem
[Projektplan](../../../../physim/Physim_Projektplan.md).
Die [Sprachdokumentation](../../../../physim/docs/language.md) beschreibt einen in C17
geschriebenen Compiler mit einem C17-Backend und nativer Übersetzung. Ihre
Versionsnummer wird getrennt von App und SDK-ABI geführt.

Eingesehen am 2026-10-06. Die Aussagen sind aus Dokumentation übernommen;
Implementierung und Funktionsgleichheit wurden für diese Notiz nicht neu geprüft.

## Bedeutung für die Arbeit

Bei einer neuen Physikfunktion zunächst klären, welcher Teil in den Kern gehört,
wie C und die Physim-Sprache darauf zugreifen und wie Messwerte beziehungsweise
Berichte entstehen. Danach die App-Anbindung betrachten. Die Abnahme sollte
zum betroffenen Arbeitsablauf passen.

Diese Arbeitsfolge ist eine Ableitung aus dem Projektaufbau. Konkrete
Anforderungen und Tests bestimmt weiterhin der jeweilige Projektauftrag.
