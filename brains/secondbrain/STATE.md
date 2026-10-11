# SecondBrain: aktueller Stand

Stand: 12. Oktober 2026. Installiert ist 0.9.69. Der letzte vollständige lokale
Release-Lauf und die bestätigte Windows-/Linux-CI gehören zur vorherigen 0.9.68.
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

Installiert **0.9.69 / 8d74c8d72d6a**, sauberer Intel/macOS-Release auf
macOS 14.6.1/23G93. 98 Dateien paketgleich. Paketprüfungen: 126 Desktop-,
167 Tastatur-, 75 Sicherungsbedingungen sowie Einstellungen, CLI, Lizenzen,
Laufzeit und Versionsidentität. Echter 68→69→68-Wechsel erhält 20 Testdateien und
Einstellungen; Sicherung/Wiederherstellung bestehen. [Nachweis](../../docs/SYSTEM_KONTRASTFARBEN.md).

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

Für 0.9.68 bestehen 84/84 lokale Release-Tests (707,14 s); die damalige
Build- und installierte Binärdatei waren bytegleich. Alle vier Windows-/Linux-Desktopjobs zu
0b0fdac/CI38181757771 einschließlich beider Release-Pakete bestehen. Mac-CI queued;
lokale Intel-Abnahme separat. Server 2025/Xvfb sind keine menschlichen
Windows-11-/Wayland-/Screenreaderabnahmen.

## Vor 1.0 noch offen

Mindest-OS/frische Nutzerrechner und aktuelles ARM64; menschliche VoiceOver/NVDA/
Orca-/IME-/Dialogbedienung; echte Monitore/Dichtewechsel, Trackpads, schwächere
Geräte und Langzeit; volle Windows-/Linux-Volumes und physische Persistenz;
vollständiger Runtime-/Lizenzabgleich, vertraulicher Sicherheitskanal, Signierung
und dauerhafte aktuelle Pakete. [Abnahmekriterien](../../docs/RELEASE_ABNAHME.md).

Mindestversionsvorschlag: macOS 14, Windows 11 25H2, Ubuntu 24.04 LTS;
noch keine endgültige Zusage. Apple-Developer-Konto/Windows-Zertifikat fehlen.
Endprodukttexte erst abschließend beim vollständigen Produkt bereinigen.
**1.0 nur nach ausdrücklicher Nutzerfreigabe.**

## Nächster Schritt

0.9.69-CI38184984352: Windows Debug/Release und Linux Release samt Release-Paketen
bestehen. Linux Debug scheitert bei der Textvorbereitung; Palette besteht, Mac queued.
0.9.70 gibt Windows-Schemanamen frei; neue native Allokatorprüfung noch offen.
0.9.71 korrigiert einen lokal reproduzierten FreeType-Listenfehler beim parallelen
Schließen privater Fonts. Alter Test hängt, alte ASan-Vergleichsfassung zeigt
Use-after-free; korrigierter 1024er-Test und acht lokale Tests bestehen.
[Schriftlebensdauer](../../docs/SCHRIFT_LEBENSDAUER.md),
[Kontrastfarben](../../docs/SYSTEM_KONTRASTFARBEN.md). Installiert bleibt 0.9.69.
Nächster Schritt: vollständiger aktueller lokaler Lauf, native CI und neue Pakete;
der frühere Linux-Absturz ist damit noch nicht als nativ behoben abgenommen.
Weitere technische Abnahmen und benötigte reale Umgebungen anhand der
Abnahmeübersicht klären.

## Quellen und Historie

[Abnahmeübersicht](../../docs/RELEASE_ABNAHME.md),
[Umsetzungsstand](../../docs/STATUS.md), [Release-Liste](../../docs/RELEASE.md),
[Journal](journal/2026-10-11.md) und [archivierter Stand](archive/state-2026-10-12.md).
