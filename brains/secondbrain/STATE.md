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
das geprüfte Intel-Entwicklungspaket 0.9.5 unter dist/SecondBrain.
Die öffentliche dauerhafte Vorabversion ist weiterhin
[v0.7.3](https://github.com/Lulus792/SecondBrain/releases/tag/v0.7.3).

## Laufender Schritt 0.9.5

Titel, Leseansicht und Sternkarte verwenden gemeinsame eigene C-Regeln für
Inline-Code, Maskierungen und Verweise. Echte Ziele und optionale Titel bleiben
getrennt. Überschriftenlinks sind bedienbar, Kontextüberschriften sichtbar.
17 Kernprüfungen, erster Durchlauf mit 31 Release-Tests und abschließende
Kern-/native-/Tastaturnachprüfungen bestehen; ASan/UBSan prüft eigene Bereiche.
Regressionen scheitern mit vorheriger Leseansicht/Sternkarte. Das entpackte
Intel-Paket einschließlich Desktop, Tastatur, Sicherung, Neustart und CLI besteht.
[Vertrag](../../docs/INLINE_LINKS.md), [genaue Nachweise](../../docs/STATUS.md).

Der 0.9.4-Lauf besteht in 19 Jobs und vier Paketen; Windows Debug erreicht den
Tastatur-Timeout. Der Prüfwerkzeug-Renderer zeichnet nun den abgeschlossenen
Key-up-Zustand, während beide Phasen sämtliche Zustände berechnen. Neue
native Plattformabnahme ist erforderlich; der frühere Lauf gilt nicht als grün.

## Weiterarbeiten

0.9.5 nativ abnehmen, insbesondere Windows Debug und die geänderte Tastaturprüfung.
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
