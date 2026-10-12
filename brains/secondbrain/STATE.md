# SecondBrain: aktueller Stand

Stand: 12. Oktober 2026. Installiert ist **0.9.74 / eabf5b3ff114** auf
Intel/macOS 14.6.1. Entwicklungsbuild zu95341ae besteht lokal mit92 Tests;
Windows/Linux zu dc3ee0e bestehen in Debug/Release mit jeweils92 Tests und
beiden Release-Paketen, einschließlich Linux-Wayland. Mac-CI wartet auf Runner.
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

- [Signierungsvorbereitung](../../docs/SIGNIERUNG.md): neue Paketkopie,
  native Signatur-/Paketdatenprüfung und getrennte Freigabe.88 Python-Tests
  (eine übersprungen) und echter Mac-Ad-hoc-/Hardened-Runtime-Test bestehen,
  einschließlich Ressourcenänderungsabweisung und Metal/Cocoa-Start.
  Zertifikate, Windows-Signierung und tatsächliche Notarisierung bleiben offen.
- Vollständiger Intel/macOS-Lauf zu95341ae:92/92 Tests in717,26s;501 Repository-
  Dateien und beide Programme unverändert. Echte HFS+-Backup-/Schreibfälle auf
  entferntem32MiB-Testimage bestehen. [Ergebnis und Scope](../../docs/evidence/mac-95341ae-tests.json).
  Entwicklungsbuild, keine neue Installation oder menschliche/ARM64-Abnahme.
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
- Installierter [Linux-Startobjektbestand](../../docs/LINKER_BESTAND.md): neuer
  Entwicklerprüfer für Paket-/Quellversionen und originale Copyrightdateien,
  an die Programm-/Kartenhashes gebunden. Vier lokale Verträge, 68 Python-Tests
  (eine übersprungen) und je fünf Beiträge in tatsächlichen Karten geprüft.
  Native Paketabfrage in Linux Debug/Release abgeschlossen: fünf Dateien, zwei
  Original-Copyrighttexte; Berichts-/Paketprogrammhashes geprüft. CI38201394939
  Windows/Linux jeweils91 Tests und beide Release-Pakete erfolgreich.
- Windows-Runtime-Dateien: geladene CRT-/uuid-Dateien über MSVC-Log und
  Kartenmitglieder erfassen. Vier neue Verträge, zwei CTest-Gruppen und72 Python-
  Tests (eine übersprungen) lokal geprüft. Erste native Ausführung verweigert
  korrekt einen Nachweis aus bloßen Suchzeilen; vollständige Linker-/normale
  MSBuild-Ausgabe korrigiert. CI38202931306 Windows/Linux jeweils92 Tests und
  beide Pakete erfolgreich. Vier Windows-CRT-/uuid-Dateien mit tatsächlichen
  Herkunftspfaden gehasht; Bericht und Log an Paketprogrammhashes nachgeprüft.
- glibc-Startdateien: passende Ubuntu-Original-/Debian-Archive gegen publizierte
  SHA256 geprüft; besondere Linkerlaubnis in allen drei x86-64-Dateien vorhanden,
  kein erwähnender Patch in116 Quilt-Einträgen. Keine reproduzierte Übersetzung
  und keine geprüfte PGP-Signatur; Originale unter build/glibc-source-review/.
- Innere Windows-AccessKit-DLL: optionaler abschließender Cargo-Linkeraufruf
  erzeugt jetzt ebenfalls eine Karte mit DLL-/Kartenhashes. Cargo--lib-Vertrag
  lokal mit cdylib/staticlib, Pfad mit Leerzeichen/Umlaut und Hashbindung geprüft;
 72 Python-Tests (eine übersprungen) bestehen. CI38204329310 Windows/Linux
  Debug/Release erfolgreich; Windows-DLL-Karte SHA-geprüft und Release-DLL-Hash
  passt exakt zur Paketdatei. Benannt:29 libcmt-,21 libvcruntime-,99 libucrt-
  Mitglieder. Eigene DLL-SDK-Pfade und vollständige Rust-Quellzerlegung offen.
  Direkte Linkereingabepfade im optionalen MSVC-Nachweis-Build ergänzt, mit
  eigener DLL-/Listenhashbindung; sechs lokale CMake-Bedingungen und72 Python-
  Tests (eine übersprungen) bestehen. CI38205225168 Windows/Linux Release und
  Pakete erfolgreich; neue Windows-DLL-/Karten-/Listenhashes passen zum Paket.
  Direkter libcmt-Pfad passt zum SDK-Dateihash desselben Builds. Beobachtete
  Liste enthält keine direkten libvcruntime-/libucrt-Pfade; diese bleiben offen.
  Ergänzender Link-Repro-Bytevergleich vorbereitet: genau eine SDK-Kopie je
  DLL-Kartenbibliothek und Übereinstimmung mit der SDK-Inventur verlangt.
  78 Python-Tests (eine übersprungen) und CTest-Inventurvertrag bestehen;
  tatsächliche neue Windows-Ausführung noch offen. SDK-Kopien werden nicht verteilt.
  Erste Windows-Debug-Ausführung scheitert: App/CLI laden Debug-CRT, Cargo-DLL
  Release-CRT. Vergleich auf tatsächlich gelesene Release-/Debug-Gegenstücke
  im beobachteten SDK-Verzeichnis erweitert; keine Ersetzung durch Debug-Bytes.
  80 Python-Tests (eine übersprungen) und CTest-Inventurvertrag bestehen.
  CI38206935201 Windows/Linux Debug/Release und Pakete erfolgreich. Geprüfter
  Debug-Nachweis bestätigt92 Tests und getrennte Debug-App-/Release-DLL-SDKs.
  Release zu17e7a93 inzwischen vollständig bestätigt:92 Tests, Paket-DLL-
  Hash und alle drei SDK-Bytevergleiche passen; keine SDK-Kopien in Downloads.
- [Wayland-Startprüfung](../../docs/WAYLAND_START.md) für gebaute App und Paket
  besteht zu dc3ee0e nativ unter Linux Debug/Release und im Release-Paket:
  Weston13/Pixman, OpenGL/wayland, vier Frames,62 Module,13 erhaltene Dateien;
  Paket-App-Hash stimmt mit beiden Release-Berichten überein. Drei ZIPs SHA-
  geprüft; je92 Tests.82 Python-Tests (eine übersprungen), vier lokale Prozess-/
  Socket-Fälle und vorhandener Metal/Cocoa-Start bestehen. Menschliche Abnahme offen.
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

Die [konsolidierte Lizenzinventur](../../docs/LIZENZ_INVENTUR.md) trennt nun
technische Quellen-/Bytebelege und noch offene konkrete Verteilungsbedingungen.
Geräteliste für reale Abnahmen abwarten; macOS-Runner/ARM64, menschliche Bedienung
und Signierung benötigen externe Voraussetzungen. Keine1.0-Freigabe daraus ableiten.
Sicherheitskanal nach GitHub-Anmeldung aktivieren.
Geräte/Konten und menschliche Bediennachweise werden durch weitere kleine
Codeänderungen oder Statusnotizen nicht ersetzt.

## Quellen und Historie

[Abnahmeübersicht](../../docs/RELEASE_ABNAHME.md),
[Umsetzungsstand](../../docs/STATUS.md), [Release-Liste](../../docs/RELEASE.md),
[Journal](journal/2026-10-12.md), [Quellen](SOURCES.md) und
[archivierter Stand](archive/state-2026-10-12.md).
