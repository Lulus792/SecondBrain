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

## Geprüfter Paketstand 0.9.14

[Zeichenreferenzen](../../docs/ENTITIES.md) sind im eigenen C-Leser implementiert:
2.125 festgelegte Namen, numerische Angaben, stabile Ausgabestile, erhaltene
Literalbereiche und Quellen, dekodierte lokale Ziele und richtige Sternkarten-
beziehungen. Math-Fallback ersetzt bestätigte Kästchen; Originaldaten/-fonts
und Lizenzen sind zugeordnet. 36 Release-Tests, abschließende Kern-/native
Nachprüfungen und neun Python-Strukturtests bestehen lokal. Eigene Kern-
ASan/UBSan mit 1.194.343 Assertions besteht; letzte native Sanitizer-Nachprüfung
besteht mit 393 Assertions. UI-Bericht SBUI-048/049. Neues Intel/macOS-Paket besteht einschließlich
Desktop/Tastatur/Sicherung/Neustart/CLI; dist enthält 0.9.14, Build ddd2186d2ab9.
Das eigene Gedächtnis wurde geladen und die Darstellung betrachtet.
Lauf 37639562794 zu ddd2186 besteht in allen zwölf Windows-/Linux-Jobs
einschließlich Debug/Release und entpackter Pakete. Neue Mac-CI noch nicht
vollständig abgeschlossen; daraus keine ARM64-Gesamtabnahme ableiten.

Vorherige Windows-/Linux-Abnahme 37631991014 zu 9c38604 besteht in zwölf Jobs,
einschließlich Debug/Release und Release-Paketen. Neue Mac-CI separat übernehmen;
keinen Plattformnachweis der neuen 0.9.14 daraus ableiten.

## Neue Parserarbeit 0.9.15

E-Mail-Autolinks und innerer Linkvorrang sind implementiert; automatische
Empfänger werden vor OS-Übergabe kodiert. 37 Release-Tests, 19 Originalfälle,
1.000 Adressfälle, 1.194.616 eigene Kern- und 401 native Sanitizerassertions
bestehen lokal; Raster betrachtet. UI-Bericht SBUI-050/051. Reale Mail-App
nicht bedient. Paket-/Plattformabnahme folgt; dist bleibt bis dahin 0.9.14.
Referenzlinks sind recherchiert, die dokumentweite C-Umgebung bleibt nächste Arbeit.

## Nächste Arbeit und Grenzen

Verbleibende macOS-CI-Nachweise übernehmen. Danach Container-/
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
