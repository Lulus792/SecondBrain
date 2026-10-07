# SecondBrain: aktueller Stand

Stand: 7. Oktober 2026. Maßgebliche Originale:
[Umsetzungsstand](../../docs/STATUS.md), [Plattformnachweise](../../docs/PLATTFORMEN.md)
und [Release-Aufgaben](../../docs/RELEASE.md). Historische Schritte stehen im
[Journal](journal/2026-10-07.md).

## Produkt und geprüfter Stand

Eigene C17-Desktop-App für Mensch und KI: Projekte und Notizen anlegen,
lesen, bearbeiten, suchen und archivieren; Quellen lesen und gespeicherten
Projektkontext kopieren. Lumen-Sternkarte, eigene Glasdarstellung und Icons,
direkte Pfeilnavigation, Raumfahrt, begrenztes weiches Scrollen und große
Leseansicht sind implementiert. Entwürfe und erkannte Konflikte bleiben geschützt.

Eigene Inhaltsarchive bieten Sicherung, Prüfung und Wiederherstellung in einen
freien Projektordner. Einstellungen und Arbeitsstand bleiben gespeichert.
Native Adapter, bekannte Systemvorgaben und Schrift-Fallback sind angebunden.
[Verträge und Grenzen](SOURCES.md).

Die [0.9.3-Abnahme zu 3f3d3a0](https://github.com/Lulus792/SecondBrain/actions/runs/37550880880)
besteht mit 20 Jobs und vier entpackten Paketen für Windows x64, Linux x64,
macOS ARM64 und Intel. Sie umfasst 17 lesbare Lizenzressourcen, Tastaturwege,
Entwurfsschutz und bestehende native Provider-/Clientprüfungen. Lokal liegt
das geprüfte Intel-Entwicklungspaket 0.9.4 unter dist/SecondBrain.
Die öffentliche dauerhafte Vorabversion ist weiterhin
[v0.7.3](https://github.com/Lulus792/SecondBrain/releases/tag/v0.7.3).

## Laufender Schritt 0.9.4

Titel und Leseansicht benutzen gemeinsame schreibgeschützte C-Blockregeln für
Überschriften, Code und Absätze. Editor und gespeicherte Originalbytes bleiben
erhalten. 16 Debug-Kernprüfungen, der erste Durchlauf mit 30 Release-Tests,
abschließende Kern-/native Nachprüfungen und gezieltes ASan/UBSan bestehen.
Die neue Rollenprüfung scheitert mit der vorherigen Leseansicht.
[Markdown-Vertrag](../../docs/MARKDOWN.md), [genaue Nachweise](../../docs/STATUS.md).
Das entpackte Intel-Paket besteht einschließlich Desktop, Tastatur, Sicherung,
Einstellungsneustart und CLI. Neue native Plattformabnahme folgt nach dem Push.

## Weiterarbeiten

0.9.4 vollständig auf den nativen Plattformen und im entpackten Paket abnehmen.
Danach vollständige Container-/Inline-Regeln und Listen-/Tabellensemantik,
Unicode-Textgeometrie/IME, Support-/Lizenzzuordnung sowie übrige Release-Aufgaben
weiter umsetzen. Tatsächliche native Dialog- und menschliche VoiceOver/NVDA/Orca-
Bedienung, Geräte-/Langzeitprüfungen und weitere volle Dateisysteme bleiben offen.
CI-Nachweise belegen keine vollständige menschliche oder physische Geräteabnahme.

Der vollständige Auftrag vor 1.0 bleibt aktiv. Eigener Code: MIT.
Apple-Developer-Konto und Windows-Signaturzertifikat fehlen.
1.0 wird ausschließlich nach ausdrücklicher Nutzerfreigabe gesetzt.
Die abschließende Bereinigung der Produkttexte folgt auf Nutzerwunsch erst,
wenn das Produkt vollständig ist. Originalnachweise bleiben erhalten.
