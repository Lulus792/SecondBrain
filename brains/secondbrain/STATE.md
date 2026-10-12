# SecondBrain: aktueller Stand

Stand: 12. Oktober 2026. Installiert ist **0.9.74 / eabf5b3ff114** auf
Intel/macOS 14.6.1. 0.9.74 besteht unter Windows/Linux in Debug/Release mit
jeweils 90 Tests und beiden Release-Paketen; Mac-CI wartet auf Runner.
1.0 ist nicht freigegeben. Originale und Historie stehen am Ende dieser Notiz.

## Produkt und installierte Fassung

Eigene C17-Desktop-App für Mensch und KI: Projekte/Notizen anlegen, lesen,
bearbeiten, suchen, archivieren, sichern; Quellen lesen und Kontext kopieren.
Lumen-Sternkarte, Glaskarten, eigene Icons, direkte Pfeilwahl mit Startfokus,
Kamerafahrt, weiches Scrollen und große Leseansicht sind implementiert.
Vorlagen und Updates ersetzen vorhandenes Wissen nicht automatisch.
Ungespeicherte Entwürfe und erkannte externe Konflikte bleiben geschützt.
Externe Bibliotheken nur für UI; eigener Code MIT.

98 installierte Dateien paketgleich. Paket: Desktop 126/Tastatur 167/Sicherung 75,
Einstellungen, CLI, Lizenzen, Laufzeit und Version. Echter 73→74→73-Wechsel erhält
20 Dateien und Einstellungen; Sicherung/Wiederherstellung bestehen. 22 eigene
Gedächtnisdateien beim Snapshot und normalen installierten Start unverändert.
[Abnahme](../../docs/RELEASE_ABNAHME.md).

## Neueste geprüfte Änderungen

- [Sternbeschriftungen](../../docs/STERNBESCHRIFTUNGEN.md): symmetrischer Abstand
  außerhalb des Fokusrings und tatsächliche Schriftzeilenmitte. 108 neue
  Bedingungen, negative Regression, acht gezielte CTest-Gruppen und betrachtete
  Raster bestehen lokal. Saubere 0.9.74: Versionen/normaler Start in 1,62 s,
  entpacktes Intel-Paket und Installation geprüft. 170 Produktinputs/Identitäten
  während der Paketprüfung unverändert. CI38197916019 Windows/Linux vollständig
  erfolgreich; SHA-geprüfte native Paketberichte/Archive stimmen überein.
- [Linkerbestand](../../docs/LINKER_BESTAND.md): bytegleiche Diagnosekarte zur
  installierten Mac-App; optionaler Buildweg erzeugt App-/CLI-Karten mit Hash-
  Identitäten außerhalb der Pakete. Neue CI38199346371 Windows/Linux vollständig
  erfolgreich: jeweils90 Tests, beide Release-Pakete; Karten und tatsächliche
  Paketprogramme stimmen über Hashes überein. Konkrete SDK-/Startobjektquellen
  und innere Rust-Anteile sind der weitere Lizenzabgleich; Mac-CI queued.
- [Startdiagnose](../../docs/LAUFZEIT_START.md): tatsächliche normale Rendererwahl,
  vier Frames, sichtbares Fenster und beobachtete Module. Dadurch echter Windows-
  Startfehler gefunden: Accessibility muss vor dem ersten sichtbaren Fenster
  eingerichtet werden. Reihenfolge korrigiert; Prüfer normalisiert ausschließlich
  CRLF der MSVC-Versionsausgabe gegenüber LF im JSON, strikte Sichtbarkeit bleibt.
- Lokal 89/89 zu e66e023 (517,45 s); Binärhash/211 Inputs unverändert. Danach nur
  Prüfer/CI/Dokumentation geändert; Produktinputs unverändert. Saubere 15e63b4-
  Fassung bestätigt Versionen/normalen Start in 1,22 s; Paket/Installation gehören
  zu 15e63b4. ASan/UBSan für korrigierte sichtbare Metal-Starts im dokumentierten
  Umfang; externe UI nicht instrumentiert, Leaks aus, kein TSan.
- CI38195529730: Windows/Linux Debug/Release und Release-Pakete erfolgreich.
  SHA-geprüfte Paketberichte: Direct3D11/Windows 52 Module, OpenGL/X11 65 Module;
  jeweils sichtbares Fenster, vier Frames, 13 Dateien erhalten. Mac-CI queued.
  Installierter Intel-Mac: Metal/Cocoa 394 Module.
- [Volle Volumes](../../docs/VOLLER_SPEICHER.md): Backup und fünf Schreib-/Modellfälle
  auf HFS+/NTFS/ext4 im dokumentierten Umfang bestätigt. Weitere Dateisysteme und
  physische Persistenz bleiben offen.
- [Laufzeitquellen](../../docs/RUST_RUNTIME_NACHWEIS.md): aktuelle Windows-/Linux-
  Compiler im vorhandenen konservativen Rust-Umfang abgedeckt; Lizenztexte mit
  gesonderten Windows-Zeilenenden geprüft. [Transitive ELF-Anforderungen](../../docs/PAKET_LAUFZEIT.md)
  mit tatsächlichen Dateien je Programm nativ bestätigt.
- [Einstellungsfehler](../../docs/EINSTELLUNGEN_BEENDEN.md),
  [Schriftlebensdauer](../../docs/SCHRIFT_LEBENSDAUER.md),
  [Materialrückfall](../../docs/MATERIAL_RUECKFALL.md) und
  [Übergangsbild](../../docs/UEBERGANG_BILD.md) im jeweiligen begrenzten Umfang geprüft.

## Vor 1.0 noch offen

Mindest-OS/frische Nutzerrechner und aktuelles ARM64; menschliche VoiceOver/NVDA/
Orca-/IME-/Dialogbedienung; echte Monitore/Dichtewechsel, Trackpads, schwächere
Geräte und Langzeit; weitere Dateisysteme/physische Persistenz; vollständiger
Runtime-/SDK-/Lizenzabgleich, vertraulicher Sicherheitskanal, Signierung und
dauerhafte aktuelle Pakete. [Kriterien](../../docs/RELEASE_ABNAHME.md).

Mindestversionsvorschlag: macOS 14, Windows 11 25H2, Ubuntu 24.04 LTS;
noch keine endgültige Zusage. Apple-Developer-Konto/Windows-Zertifikat fehlen.
Sicherheitskanal gewählt: GitHub Private Vulnerability Reporting; Aktivierung
und Erreichbarkeit werden noch geprüft.
Sicherheitsrichtlinie und Verweis aus den Fehlerformularen vorbereitet. Der
verfügbare Browser ist nicht angemeldet; der Connector bietet die Aktivierung
nicht an. GitHub-Anmeldung im Browser für den nächsten Schritt erforderlich.
Endprodukttexte erst beim vollständigen Produkt abschließend bereinigen.
**1.0 nur nach ausdrücklicher Nutzerfreigabe.**

## Nächster Schritt

Quellen der konkret eingebundenen Runtime-/SDK-/Startobjekte abgleichen.
Sicherheitskanal nach GitHub-Anmeldung aktivieren.
Geräte/Konten und menschliche Bediennachweise werden durch weitere kleine
Codeänderungen oder Statusnotizen nicht ersetzt.

## Quellen und Historie

[Abnahmeübersicht](../../docs/RELEASE_ABNAHME.md),
[Umsetzungsstand](../../docs/STATUS.md), [Release-Liste](../../docs/RELEASE.md),
[Journal](journal/2026-10-12.md), [Quellen](SOURCES.md) und
[archivierter Stand](archive/state-2026-10-12.md).
