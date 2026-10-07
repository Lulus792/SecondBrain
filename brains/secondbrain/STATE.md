# SecondBrain: aktueller Stand

Stand: 7. Oktober 2026. Maßgeblich sind [STATUS](../../docs/STATUS.md),
[Plattformnachweise](../../docs/PLATTFORMEN.md) und [Release-Aufgaben](../../docs/RELEASE.md).
Details vergangener Schritte bleiben im [Journal](journal/2026-10-07.md).

## Produkt

Eigene C17-Desktop-App für Mensch und KI: Projekte/Notizen verwalten, Wissen lesen,
bearbeiten, suchen und archivieren; Quellen lesen und gespeicherten KI-Kontext
kopieren. Lumen-Sternkarte, eigene Glaskarten und Icons, direkte Pfeilnavigation,
Raumfahrt, begrenztes weiches Scrollen und große Leseansicht sind implementiert.
Entwürfe und erkannte Konflikte bleiben geschützt. Sicherung/Wiederherstellung,
Einstellungen, eigener Markdown-Blockleser und vollständige Graphem-Eingaben
gehören zum bisherigen geprüften Umfang. [Verträge und Quellen](SOURCES.md).

## Aktueller Arbeitsschritt 0.9.13

Windows Debug/Release der 0.9.12-Abnahme scheitert am fehlenden FindText-Bereich;
32 andere UI-Tests bestehen. Linux Debug/Release einschließlich Stilabfragen
und Release-Paket besteht. [Windows-UIA-Textsuche](../../docs/UIA_TEXTSUCHE.md):
Adapterbefund bestätigt, kleine gehashte UI-Ergänzung und fester DLL-Quellbuild
vorbereitet. Lokale Quellen-/Lockfile-/Wiederholungsprüfungen und vier Mac-
Nachprüfungen bestehen. Neue Windows-Kompilierung und native Abnahme folgen.
Erster Quellbuild 37628165496 stoppt unter Windows schon bei Konfiguration;
Ursache inzwischen belegt: native CRLF-Ausgabe statt festgelegter LF-Bytes.
Vorbereitung erzwingt nun LF und erhält strenge Hashprüfung; vier Quellenfälle
bestehen lokal. Windows-Konfiguration besteht nun. Zwei vom Windows-Compiler gemeldete falsche
API-Aufrufe sind nach den Originalquellen korrigiert; neue Kompilierung und native
Clientprüfung folgen. Intel-Paket 0.9.13 besteht; kein Windows-Nachweis.
Keine bestätigte Windows-Behebung vorwegnehmen. UI-Bericht SBUI-047.
Dist enthält weiter geprüftes 0.9.12, Build 4c4a51054334.

## Nächste Arbeit und Grenzen

Neue Paket-/Plattformnachweise übernehmen. Danach verbleibende Container-/
Inline-/Listenregeln, Bidi, visuelle/native Textgeometrie und IME, native Tabellen-
Matrixschnittstellen sowie übrige Release-Aufgaben umsetzen und abnehmen.
Support-/Beitrags-/Issue-Vorlagen und Updateablauf sind veröffentlicht; vertraulicher
Sicherheitskanal ist angefragt, transitive Lizenzprüfung und Wartungsverantwortung
bleiben offen. GitHub-Formularansicht verlangt eine Anmeldung und ist noch nicht abgenommen.

Menschliche VoiceOver/NVDA/Orca-, native Dialog-, Geräte-/Langzeit- und weitere
Vollvolume-Abnahmen bleiben offen. CI und Raster ersetzen diese Nachweise nicht.
Der vollständige Auftrag bleibt aktiv. Eigener Code: MIT; Signaturkonten fehlen.
1.0 und abschließende Produkttext-Bereinigung erst nach den dafür festgelegten
Voraussetzungen beziehungsweise ausdrücklicher Nutzerfreigabe.
