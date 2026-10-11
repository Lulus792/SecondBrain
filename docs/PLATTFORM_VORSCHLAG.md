# Vorschlag für die unterstützten Systeme von1.0

Stand:11.Oktober2026. Der Nutzer hat einen begründeten Vorschlag beauftragt.
Dies ist ein Prüfziel, keine bereits bestätigte Freigabe oder Kompatibilitätszusage.

| Plattform | Vorgeschlagene Untergrenze | Geplanter Paketumfang |
| --- | --- | --- |
| macOS | 14.0 | getrennte Intel-x86_64- und Apple-Silicon-arm64-Pakete |
| Windows | Windows11 25H2 | x64, zusätzlich aktuelle unterstützte Windows11-Version prüfen |
| Linux | Ubuntu24.04 LTS | x86_64, reguläre Desktopnutzung unter X11 und Wayland abnehmen |

## Begründung und tatsächliche Nachweise

**macOS:** Die aktuelle installierte Intel-App0.9.64/bef4816d3e40 deklariert in
`LC_BUILD_VERSION` tatsächlich `minos14.0`, SDK15.2. Ausgeführt ist sie auf
macOS14.6.1/23G93. Das eingebettete Minimum ist eine technische Startbedingung,
kein ausgeführter14.0-Test.14 als Zieluntergrenze erhält die vorhandene Mac-
Nutzung und benötigt keine Apple-26-Materialkomponente; die App zeichnet ihr
Glas selbst. Das neue ARM64-Paket und beide Architekturen auf der zugesagten
Untergrenze sind noch zu prüfen. Apple listet im aktuell eingesehenen September-
2026-Stand Updates für15,26 und27; diese aktuellen Systemstände gehören zusätzlich
in die Abnahme. Das ist keine Zusage über zukünftige Apple-Sicherheitszeiträume.
[Apple-Sicherheitsveröffentlichungen](https://support.apple.com/en-us/100100).

**Windows:**25H2 ist eine noch gepflegte Basis; Microsoft nennt für Home/Pro
13.Oktober2027 als Ende, für24H2 bereits14.Oktober2026. Deshalb würde ich24H2
kurz vor dessen Ende nicht zum neuen Release-Mindestziel machen. Die aktuelle
26H2-Version sollte zusätzlich geprüft werden. Quelle:
[Microsoft Windows11 Home/Pro](https://learn.microsoft.com/en-us/lifecycle/products/windows-11-home-and-pro).
Die eigenen Windows-CI-Prüfungen laufen tatsächlich auf **Windows Server2025,
10.0.26100**, nicht auf einem Windows11-Nutzerrechner. Sie belegen Compiler,
UIA-Client und Pakete in dieser Umgebung; Windows11 25H2/26H2 bleibt eine echte
Nutzerplattformabnahme. Windows10 wäre eine zusätzliche gesonderte Kompatibilitäts-
entscheidung; sein regulärer Home/Pro-Support endete am14.Oktober2025.
[Microsoft Windows10](https://learn.microsoft.com/en-us/lifecycle/products/windows-10-home-and-pro).

**Linux:** Die geprüfte CI-Basis ist **Ubuntu24.04.5,x86_64**, Xvfb/X11, mit
Debug-/Release-, AT-SPI- und Paketprüfungen. Ubuntu24.04 LTS erhält laut Canonical
Standard-Sicherheitswartung bisMai2029;26.04 LTS bisMai2031.24.04 als Untergrenze
passt zum heutigen Paketbau und hält eine länger gepflegte Basis. Die aktuelle
26.04-LTS-Desktopumgebung sollte ebenfalls geprüft werden.
[Canonical Release-Zyklus](https://ubuntu.com/about/release-cycle).
Das ist keine Zusage für jede Linux-Distribution. Ein Ubuntu-Build kann neue
Systemsymbole benötigen; glibc-/C++-/Rust-Runtimeanforderungen der tatsächlichen
Pakete müssen vor einer breiteren Distributionszusage ausgewertet werden.
Das heruntergeladene, per SHA256 verifizierte Linux-Paket0.9.64 verlangt direkt
GLIBC-Versionstags bis2.39 (App) beziehungsweise2.34 (CLI). Die App verlangt
außerdem GCC_4.2.0 und LIBDBUS_1_3. Der neue
[ABI-Nachweis](PAKET_LAUFZEIT.md#elf-anforderungen-vom-11-oktober) erfasst die
Anforderungen pro Bibliothek; transitive Systembibliotheken und dynamisch
geladene Komponenten sind damit noch nicht vollständig abgenommen.
Wayland, native Ordnerauswahl, GPU und menschliche Orca-Bedienung sind keine
bereits bestandenen Xvfb-Prüfungen.

Die bekannten SDL-Plattformen erweitern diesen Nachweis nicht: Die App verwendet
auch weitere UI-Bibliotheken, native Adapter und eigene Betriebssystem-APIs.
Eine dokumentierte SDL-Untergrenze ist daher nicht automatisch die App-Untergrenze.

## Umsetzung vor einer Zusage

- Deployment-Ziel und Binärmetadaten pro Mac-Architektur gemeinsam kontrollieren;
  auf14.0 und aktuellem gepflegtem System tatsächlich starten und bedienen.
- Entpackte Windows-x64-Pakete auf25H2 und26H2 abnehmen, einschließlich NVDA,
  nativer Dialoge, hoher Skalierung und physischer Sicherung/Wiederherstellung.
- Ubuntu24.04/26.04 als echte Desktopumgebungen prüfen: X11/Wayland, native Dialoge,
  Runtimeabhängigkeiten, verschiedene Grafikkarten und Orca.
- Aufgaben der Release-Liste und Aktualisierung ohne Wissensverlust auf frischen
  Rechnern abnehmen. Erst danach die unterstützten Versionen verbindlich nennen.

ARM64 für Windows/Linux, weitere Distributionen und macOS vor14 sind mögliche
weitere Ziele. Sie sind in diesem Vorschlag nicht als nachgewiesen zugesagt.
Die vorgeschlagene Tabelle setzt keine1.0-Versionsnummer und verändert noch keine
Build-/CI-Untergrenzen. Die Veröffentlichung1.0 bleibt ausdrücklich dem Nutzer vorbehalten.

Originale: `build/creation-icons-verification/platform-*.log` und
`macos-binary-version.log`; CI38170374268 zu798b84e. Release-Belege und offene
Prüfungen stehen in [RELEASE](RELEASE.md), [DISTRIBUTION](DISTRIBUTION.md) und STATUS.
