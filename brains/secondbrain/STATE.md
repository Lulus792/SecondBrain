# SecondBrain: aktueller Stand

Stand: 12. Oktober 2026. Installiert ist 0.9.72/c40c9fd47ce1. Lokal und in den
vier Windows-/Linux-Desktopjobs bestehen jeweils 88 Tests.
1.0 ist noch nicht freigegeben. Die maßgeblichen Quellen und die Historie
stehen am Ende dieser Notiz.

## Produkt und installierte Fassung

Eigene C17-Desktop-App für Mensch und KI: Projekte/Notizen anlegen, lesen,
bearbeiten, suchen, archivieren, sichern; Quellen lesen und Kontext kopieren.
Lumen-Sternkarte, Glaskarten, eigene Icons, direkte Pfeilwahl mit Startfokus,
Kamerafahrt, weiches Scrollen und große Leseansicht sind implementiert.
Vorlagen und Updates überschreiben vorhandenes Wissen nicht automatisch.
Ungespeicherte Entwürfe und erkannte externe Konflikte bleiben geschützt.
Externe Bibliotheken nur für UI; eigener Code MIT.

Installiert **0.9.72 / c40c9fd47ce1**, Intel/macOS 14.6.1/23G93;
98 Dateien paketgleich. Paket: 126 Desktop-, 167 Tastatur- und 75
Sicherungsbedingungen sowie Einstellungen, CLI, Lizenzen, Laufzeit und Version.
Echter 71→72→71-Wechsel erhält 20 Testdateien und Einstellungen;
Sicherung/Wiederherstellung bestehen. 21 Gedächtnisdateien beim Öffnen unverändert.
[Nachweis](../../docs/EINSTELLUNGEN_BEENDEN.md).

## Neueste geprüfte Änderungen

- [Textvorbereitung](../../docs/TEXT_VORBEREITUNG_OPTIMIERUNG.md): genaue Breiten
  statt vollständiger Geometrie bei Umbruchversuchen; fertige Zeilen bleiben
  identisch. Drei Metal-Paare lange Notiz median 10,54 % früher vorbereitet;
  keine allgemeine Geräte-/Flüssigkeitsgarantie.
- [Materialrückfall](../../docs/MATERIAL_RUECKFALL.md): gewählte Farben und
  lesbare Karten trotz ausgewählter Materialfehler; Teilpuffer sofort frei.
- [Übergangsbild](../../docs/UEBERGANG_BILD.md): native Pixelidentität gezielt
  geprüft, zwölf schnelle Wechsel; alte Überblendung bei Resize beendet,
  Kamerafahrt und neue passende Ladeaufnahme erhalten. ASan/UBSan im dokumentierten
  Umfang; externe UI nicht instrumentiert, Leaks aus, kein TSan.

Für c40c9fd bestehen lokal 88/88 Tests (471,00 s), Binärhash und 209
Produktinputs unverändert. CI38191364672: Windows/Linux Debug/Release je 88/88,
beide Release-Pakete erfolgreich. Mac-CI queued. Server 2025/Xvfb ersetzen keine
menschlichen Windows-11-/Wayland-/Screenreaderabnahmen.

## Vor 1.0 noch offen

Mindest-OS/frische Nutzerrechner und aktuelles ARM64; menschliche VoiceOver/NVDA/
Orca-/IME-/Dialogbedienung; echte Monitore/Dichtewechsel, Trackpads, schwächere
Geräte und Langzeit; weitere Dateisysteme und physische Persistenz;
vollständiger Runtime-/Lizenzabgleich, vertraulicher Sicherheitskanal, Signierung
und dauerhafte aktuelle Pakete. [Abnahmekriterien](../../docs/RELEASE_ABNAHME.md).

Mindestversionsvorschlag: macOS 14, Windows 11 25H2, Ubuntu 24.04 LTS;
noch keine endgültige Zusage. Apple-Developer-Konto/Windows-Zertifikat fehlen.
Endprodukttexte erst abschließend beim vollständigen Produkt bereinigen.
**1.0 nur nach ausdrücklicher Nutzerfreigabe.**

## Nächster Schritt

CI38187181878 nach LF-Dateiregel: alle vier Windows-/Linux-Desktopjobs samt
Release-Paketen bestehen; Mac queued. Neuer gemeinsamer Volume-Prüfweg lokal mit
Backup-Volumeprüfung auf HFS+/ext4/NTFS bestätigt. Neu: vier echte HFS+-Fälle
für bestehende/neue Notizen und Einstellungen samt Bestandsschutz und Wiederholung
bestehen; neue native Windows-/Linux-Schreib- und Modellprüfung ebenfalls
Debug/Release erfolgreich, beide Release-Berichte per SHA256 verifiziert. Die native Windows-Allokatorprüfung
besteht bereits im vorherigen Lauf. Neues Mac-Paket, tatsächlicher Updatewechsel
und Installation sind abgeschlossen. [Schriftlebensdauer](../../docs/SCHRIFT_LEBENSDAUER.md),
[Kontrastfarben](../../docs/SYSTEM_KONTRASTFARBEN.md).
[Volume-Vertrag](../../docs/VOLLER_SPEICHER.md).
0.9.72 behandelt Einstellungs-Speicherfehler beim Beenden mit eigener Fehlerkarte,
Wiederholung, bewusster Ausnahme und Rückkehr. 93 Fehler-/Rückkehrbedingungen
und 94 Hilfe-/Auswahlbedingungen bestehen unter ASan/UBSan. Gesamt-, native
Windows/Linux-, Paket-, Update- und Installationsprüfung abgeschlossen;
menschliche Bedienprüfung bleibt offen. [Vertrag](../../docs/EINSTELLUNGEN_BEENDEN.md).
Weitere technische Abnahmen und nötige reale Umgebungen anhand der
Abnahmeübersicht klären; alle offenen Release-Anforderungen bleiben Arbeitsumfang.

Aktuelle Windows-/Linux-Pakete und native Release-Berichte zu c40c9fd per
GitHub-Digest geprüft; Programmhashes stimmen überein. Rust 1.98.1/1.99.0 ist
im vorhandenen Quellenumfang abgedeckt, je 50 Lizenztexte abgeglichen (Windows-
Zeilenenden getrennt dokumentiert). Der Entwicklerprüfer erfasst nun transitive
ELF-Anforderungen je Programm; zehn lokale Prüfungen, eine Windows-Prüfung
übersprungen. CI38192724967 Windows/Linux Debug/Release samt Release-Paketen
erfolgreich. Verifizierter Linux-Bericht enthält 22 Bibliotheken, App/CLI getrennt.
[Rust-Nachweis](../../docs/RUST_RUNTIME_NACHWEIS.md),
[Laufzeitumfang](../../docs/PAKET_LAUFZEIT.md).

0.9.73 ergänzt eine begrenzte Diagnose des normalen Renderers mit vier Frames,
Modulbeobachtung und geschütztem neuen Bericht. Lokal Metal/Cocoa, 394 Module,
13 Projektdateien unverändert; vier gezielte Gruppen und drei ASan/UBSan-Starts
bestanden. Neue vollständige/native/Paket-/Installationsprüfung folgt;
installiert bleibt 0.9.72. [Startdiagnose](../../docs/LAUFZEIT_START.md).

Der vollständige lokale 89er-Lauf zu 51b6963 besteht (461,19 s), 211 Inputs und
Binärhash unverändert. Neue native Startprüfung zeigt jedoch einen echten
Windows-Fehler: AccessKit wird nach dem ersten sichtbaren Fenster angelegt.
Windows Release 88/89; Linux Release samt Paket erfolgreich, SHA-geprüfter
Startbericht OpenGL/X11/65 Module. Reihenfolge korrigiert: zunächst verborgen,
Adapter einrichten, dann zeigen; Diagnose verlangt sichtbares Fenster. Fünf
lokale Gruppen (25,05 s) und drei ASan/UBSan-Starts bestehen. Korrigierte native
Windows-, vollständige lokale und Paket-/Installationsabnahme folgen.

## Quellen und Historie

[Abnahmeübersicht](../../docs/RELEASE_ABNAHME.md),
[Umsetzungsstand](../../docs/STATUS.md), [Release-Liste](../../docs/RELEASE.md),
[Journal](journal/2026-10-11.md) und [archivierter Stand](archive/state-2026-10-12.md).
