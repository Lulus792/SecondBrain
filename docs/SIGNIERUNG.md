# Paketkopien signieren

Stand: 12. Oktober 2026. `tools/sign_package.py` bereitet eine neue signierte
Kopie eines geprüften Pakets vor. Herausgeberzertifikate sind noch nicht vorhanden.
Der lokale Mac-Test verwendet ausdrücklich eine Ad-hoc-Signatur und ist keine
Veröffentlichungsfreigabe.

## Ablauf und Bestandsschutz

Das Werkzeug prüft die bekannten Paketeinträge, weist Symlinks und vorhandene
oder überlappende Ausgabeziele ab und kopiert das Paket in einen neuen Ordner.
Es signiert dort die Programme und prüft anschließend Signaturen, erhaltene
Daten und unveränderte Quelldateien. Fehler hinterlassen eine markierte Ausgabe
ohne erfolgreichen Ergebnisbericht. Passwörter und private Schlüssel sind keine
Kommandozeilenargumente des Werkzeugs; Zertifikate werden aus dem lokalen
Schlüsselbund beziehungsweise Zertifikatspeicher verwendet.

`signing.json` enthält Quell-/Ergebnishashes und den belegten Signierungsstatus.
`release_ready` bleibt immer false: Lizenzabschluss, Produktabnahme und
Veröffentlichungsfreigabe sind eigene Schritte. Der Mac-Bericht bestätigt keine
Notarisierung. Das Werkzeug erstellt weder Release-Tags noch GitHub Releases.

## macOS

Mit vorhandenem Developer-ID-Application-Zertifikat und bekannter Team-ID:

```sh
python3 tools/sign_package.py --platform macos \
  --source /pfad/zum/geprueften/SecondBrain \
  --output /pfad/zur/neuen/signierten-ausgabe \
  --identity 'Developer ID Application: Name (TEAMID)' --team-id ABCDE12345
```

CLI und App erhalten Hardened Runtime und sicheren Zeitstempel. Die Prüfung
verlangt Apple-Vertrauenskette, Developer-ID-Application-Zertifikat der genannten
Team-ID und die jeweilige SecondBrain-Kennung. Es werden keine zusätzlichen
Runtime-Ausnahmen aktiviert. Ad-hoc ist nur über `--ad-hoc-test` zulässig und
kann keine Herausgeberidentität behaupten.

Danach ist die tatsächliche Notarisierung noch nötig: signierte Kopie mit
`ditto -c -k --sequesterRsrc --keepParent` in ein neues ZIP packen, mit
`xcrun notarytool submit` und einem bereits eingerichteten `--keychain-profile`
einreichen, die Vorgangskennung aufbewahren und den Status Accepted prüfen.
Ein Beobachtungstimeout beendet nicht den Apple-Vorgang; anhand derselben
Kennung weiterprüfen. Anschließend App-Ticket mit `xcrun stapler staple` anheften
und mit `stapler validate`, Signaturprüfung und Gatekeeper prüfen. Stapler
unterstützt App-Bundles, keine einzelnen CLI-Dateien; deren Ticket wird online
bereitgestellt. Das endgültige Archiv und seine Hashes entstehen erst nach
diesem Schritt. Diese Service-/Herausgeberabnahme ist noch nicht ausgeführt.

## Windows

Aus einer Entwicklerkonsole mit Windows-SDK-SignTool, vorhandenem Code-Signing-
Zertifikat im Benutzer-Zertifikatspeicher und dessen SHA1-Fingerabdruck:

```powershell
python tools/sign_package.py --platform windows `
  --source C:\Pakete\SecondBrain --output C:\Pakete\NeueSignierteAusgabe `
  --certificate-sha1 CERTIFICATE_THUMBPRINT `
  --timestamp-url https://ZEITSTEMPELDIENST
```

App, CLI und AccessKit-DLL werden mit SHA256 und RFC3161-Zeitstempel signiert;
SignTool prüft anschließend alle Signaturen nach Authenticode-Richtlinie.
Ein fehlender Zeitstempel ist kein erfolgreicher Durchlauf. Der native
Windows-Zertifikats-/Zeitstempeltest steht mangels Zertifikat noch aus.

## Ausgeführte Prüfung

Sechs neue Verträge prüfen Parameter, Bestandsschutz, unbekannte Paketeinträge,
Werkzeugfehler und veränderte Paketdaten.88 Python-Tests bestehen mit einer
plattformspezifischen Überspringung.

Die echte lokale Ad-hoc-Kopie der installierten0.9.74/eabf5b3ff114 besitzt gültige
CLI-/App-Signaturen und Hardened Runtime; alle98 Quellpaketdateien bleiben
unverändert. Eine veränderte versiegelte Ressource wird nativ abgewiesen;
nach Wiederherstellung gilt die Signatur wieder. Die Ad-hoc-App erfüllt die
Developer-ID-Anforderung nicht. Normaler Start: Metal/Cocoa, vier Frames,
394 Module und13 erhaltene isolierte Projektdateien. Originale unter
`build/signing-preparation/`. Die installierte App wurde nicht ersetzt.

Grundlagen, am12.Oktober gelesen:
[Apple-Notarisierung](https://developer.apple.com/documentation/security/notarizing-macos-software-before-distribution),
[manueller Apple-Ablauf](https://developer.apple.com/documentation/security/customizing-the-notarization-workflow),
[Apple-Signaturanforderungen](https://developer.apple.com/documentation/technotes/tn3127-inside-code-signing-requirements)
und [Microsoft SignTool](https://learn.microsoft.com/en-us/windows/win32/seccrypto/signtool).
Die Apple-Markdown-Originale wurden direkt von der offiziellen Quelle gelesen.
