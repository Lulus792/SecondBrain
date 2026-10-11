# Vorschlag für die unterstützten Systeme von 1.0

Stand: 12. Oktober 2026. Der Nutzer hat einen begründeten Vorschlag beauftragt.
Die Tabelle benennt geplante Abnahmeziele. Eine verbindliche Zusage folgt erst
nach Prüfung der tatsächlichen Pakete auf diesen Systemen.

| Plattform | Vorgeschlagene Mindestversion | Architektur | Zusätzlich abnehmen |
| --- | --- | --- | --- |
| macOS | 14.0 Sonoma | Intel x86_64 und Apple Silicon arm64, getrennte Pakete | Aktuelle gepflegte macOS-Version je Architektur |
| Windows | Windows 11 25H2 | x64 | Windows 11 26H2 x64 |
| Linux | Ubuntu 24.04 LTS | x86_64 | Ubuntu 26.04 LTS; X11 und Wayland |

## Warum diese Grenzen?

**macOS 14** erhält die vorhandene Mac-Nutzung und entspricht der technischen
Untergrenze des aktuellen Intel-Pakets. Am 12. Oktober wurden an der installierten
App **0.9.69 / 8d74c8d72d6a** mit `otool -l` tatsächlich `minos 14.0` und SDK 15.2
gelesen. Ausgeführt ist diese App auf **macOS 14.6.1 / 23G93, Intel**.
Der Binärwert ist kein bestandener Test auf 14.0 und kein ARM64-Nachweis.
Die App zeichnet ihr Glas selbst; sie benötigt dafür keine native Apple-Komponente
aus macOS 26. Mindestkompatibilität und Apples Sicherheitswartung sind getrennt:
für den täglichen Einsatz empfehlen wir einen aktuell gepflegten Systemstand.
Apples [Sicherheitsveröffentlichungen](https://support.apple.com/en-us/100100)
dokumentieren einzelne Updates, keine hier zugesagte zukünftige Wartungsdauer.

**Windows 11 25H2** ist ein sinnvoller Ausgangspunkt für einen neuen Release:
Microsoft nennt für Home/Pro den **13. Oktober 2027** als Supportende. Bei 24H2
ist es bereits der **14. Oktober 2026**. Deshalb schlage ich 25H2 als
Supportuntergrenze vor. Das ist eine Produktentscheidung, keine nachgewiesene
technische Unmöglichkeit auf älterem Windows. Auch 26H2 gehört in die Abnahme.
[Microsoft-Lebenszyklus, geprüft am 12. Oktober 2026](https://learn.microsoft.com/en-us/lifecycle/products/windows-11-home-and-pro).
Die bisherigen eigenen CI-Prüfungen liefen auf **Windows Server 2025,
10.0.26100**. Ein Server-Runner ersetzt keinen Windows-11-Nutzerrechner.

**Ubuntu 24.04 LTS** passt zum bisherigen Paketbau und bietet eine länger
gepflegte Basis. Canonical nennt Standard-Sicherheitswartung bis **Mai 2029**,
für 26.04 LTS bis **Mai 2031**.
[Canonical-Release-Zyklus, geprüft am 12. Oktober 2026](https://ubuntu.com/about/release-cycle).
Die ausgeführten Linux-CI-Prüfungen nutzten **Ubuntu 24.04.5 x86_64,
Xvfb/X11**. Wayland und menschliche Orca-Bedienung sind dadurch nicht abgenommen.
Eine Linux-Zusage muss Distribution, Architektur und Laufzeitabhängigkeiten
benennen; „Linux ab Version …“ wäre ungenau.

Der frühere verifizierte Linux-Paketstand 0.9.64 verlangt direkt GLIBC-Tags bis
2.39 in der App und 2.34 im CLI. Das stützt Ubuntu 24.04 als Paketbasis;
es belegt keine vollständige Kompatibilität anderer Distributionen oder neuerer
Pakete. Transitive und dynamisch geladene Komponenten bleiben zu prüfen.
[Datierter ABI-Nachweis](PAKET_LAUFZEIT.md#elf-anforderungen-vom-11-oktober).

## Was vor der verbindlichen Zusage fehlt

- **macOS:** Deployment-Ziel der App und aller mitgelieferten Komponenten je
  Architektur kontrollieren. Aktuelle Pakete auf der Untergrenze und auf aktuellen
  gepflegten Systemen starten und bedienen, einschließlich VoiceOver und IME.
- **Windows:** Entpackte Pakete auf 25H2 und 26H2 auf frischen Nutzerrechnern
  prüfen, einschließlich NVDA, nativer Dialoge, Kontrastthemen und hoher Skalierung.
- **Linux:** Ubuntu 24.04 und 26.04 als echte Desktopumgebungen mit X11 und
  Wayland prüfen, einschließlich nativer Dialoge, Orca und Laufzeitabhängigkeiten.
- **Alle Systeme:** Projekt und Notiz erstellen, bearbeiten, suchen, archivieren,
  sichern und wiederherstellen; Paketwechsel ohne Wissensverlust und den
  vollständigen Tastaturablauf ausführen. Weitere Kriterien stehen in der
  [Release-Abnahme](RELEASE_ABNAHME.md).

Windows/Linux ARM64, weitere Distributionen und macOS vor 14 sind zusätzliche
mögliche Ziele. Dafür gibt es hier noch keine Zusage. Dokumentierte Mindestwerte
einzelner UI-Bibliotheken bestimmen nicht automatisch die Untergrenze der App.

Der Vorschlag verändert weder Build-Ziele noch CI-Runner oder Versionsnummern.
1.0 wird erst nach ausdrücklicher Nutzerfreigabe veröffentlicht.
Historische Plattformoriginale: `build/creation-icons-verification/platform-*.log`,
`macos-binary-version.log` und CI38170374268 zu 798b84e. Aktuelle Nachweise:
[Abnahmeübersicht](RELEASE_ABNAHME.md), [Distribution](DISTRIBUTION.md)
und [Umsetzungsstand](STATUS.md).
