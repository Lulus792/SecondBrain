# WHATWG-Zeichenreferenzen

Original: https://html.spec.whatwg.org/entities.json, gelesen 7. Oktober 2026.
SHA-256: `d741d877ac77c4194c4ad526b5b4a19aef8dfe411ab840a466891cdbb9f362e6`.
Die Datei bleibt unverändert. tools/make_entity_data.py prüft diesen Hash und
erzeugt eine eigene C-Tabelle mit allen 2.125 Semikolon-Namen. Namen ohne
Semikolon sind im Markdown-Vertrag nicht unterstützt. Parser bleibt eigener C-Code.

Copyright WHATWG (Apple, Google, Mozilla, Microsoft). Originale Lizenz:
https://github.com/whatwg/html/blob/main/LICENSE, SHA-256
`85dc6f5ccb57a6fe8c33d158f9fc8fc7ee5655a5d3db2cdd131c6a3d0f48a864`.
Dokumentmaterial ist CC BY 4.0; in Quellcode übernommene Teile sind BSD 3-Clause.
Der unveränderte vollständige Lizenztext liegt unter ../licenses/WHATWG.txt und
wird in Paket und App-Lizenzansicht mitgeführt. Generator wird nur für die
Entwicklung benötigt; Laufzeit und normaler Build benutzen die eingecheckte Tabelle.
