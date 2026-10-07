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

## Vorheriger Plattformstand 0.9.14

[Zeichenreferenzen](../../docs/ENTITIES.md) sind im eigenen C-Leser implementiert:
2.125 festgelegte Namen, numerische Angaben, stabile Ausgabestile, erhaltene
Literalbereiche und Quellen, dekodierte lokale Ziele und richtige Sternkarten-
beziehungen. Math-Fallback ersetzt bestätigte Kästchen; Originaldaten/-fonts
und Lizenzen sind zugeordnet. 36 Release-Tests, abschließende Kern-/native
Nachprüfungen und neun Python-Strukturtests bestehen lokal. Eigene Kern-
ASan/UBSan mit 1.194.343 Assertions besteht; letzte native Sanitizer-Nachprüfung
besteht mit 393 Assertions. UI-Bericht SBUI-048/049. Neues Intel/macOS-Paket besteht einschließlich
Desktop/Tastatur/Sicherung/Neustart/CLI; der damalige Build ist ddd2186d2ab9.
Das eigene Gedächtnis wurde geladen und die Darstellung betrachtet.
Lauf 37639562794 zu ddd2186 besteht in allen zwölf Windows-/Linux-Jobs
einschließlich Debug/Release und entpackter Pakete. Neue Mac-CI noch nicht
vollständig abgeschlossen; daraus keine ARM64-Gesamtabnahme ableiten.

Vorherige Windows-/Linux-Abnahme 37631991014 zu 9c38604 besteht in zwölf Jobs,
einschließlich Debug/Release und Release-Paketen. Neue Mac-CI separat übernehmen;
keinen Plattformnachweis der neuen 0.9.14 daraus ableiten.

## Vorheriger Paketstand 0.9.15

E-Mail-Autolinks und innerer Linkvorrang sind implementiert; automatische
Empfänger werden vor OS-Übergabe kodiert. 37 Release-Tests, 19 Originalfälle,
1.000 Adressfälle, 1.194.616 eigene Kern- und 401 native Sanitizerassertions
bestehen lokal; Raster betrachtet. UI-Bericht SBUI-050/051. Reale Mail-App
nicht bedient. Neues Intel/macOS-Paket besteht; dist enthält 0.9.15,
Build 9e6cd5675f78. Eigenes Gedächtnis geladen, Raster betrachtet.
Quellcommit 9e6cd56 ist lokal gesichert; vier GitHub-Pushes scheiterten damals mit
serverseitigem Internal Server Error. Der nachfolgende erfolgreiche Push enthält diese Commits.
Referenzlinks wurden danach in der folgenden Version integriert.

## Neue Arbeit 0.9.16

Gemeinsame dokumentweite Referenzen sind in Titel/Reader/Tabelle/native Inhalte/
Graph integriert. Vollständige Standard-Unicode-Faltung, erste Definition und
Originalbytes geprüft. Scrollschiene/Fokusrand sind mit festem Inhaltsrand
getrennt; Raster sowie 100/200-Prozent-Geometrie betrachtet. Alle 40 abschließenden
Release-Tests, 433 native Sanitizerassertions, 6.030 Referenz-/1.187.526 eigene
Inlineassertions, 81 Originalfälle und 1.606 Original-Mappings bestehen lokal.
UI-Bericht SBUI-052/053. Neues Intel/macOS-Paket besteht und ist installiert;
dist enthält 0.9.16, Build b19b47624512. Eigenes Gedächtnis geladen und
Vorschau betrachtet. Normaler Push a57bdfc..b19b476 erfolgreich; auch vorherige
Commits sind veröffentlicht. Neue CI 37646348381 läuft: C17/Python für
Windows/Linux bestehen, Desktop-/Paket-/Mac-Nachweise noch nicht abgeschlossen.

## Nächste Arbeit und Grenzen

Neue CI-Nachweise übernehmen. Danach vollständige Container-/
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
