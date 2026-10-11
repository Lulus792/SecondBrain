# Windows-Kontrastfarben in der eigenen Oberfläche

Stand: 12. Oktober 2026. Umsetzung ab 0.9.69.

## Grundlage vor der Umsetzung

Gelesen am 12. Oktober: Microsofts
[GetSysColor-Vertrag](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getsyscolor)
und [Kontrastthemen](https://learn.microsoft.com/en-us/windows/apps/design/accessibility/high-contrast-themes).
Die aktuellen Rollen paaren Fenstertext mit Fensterhintergrund, Schaltflächentext
mit Schaltflächenhintergrund und Auswahltext mit Auswahlhintergrund. Eigene Farben
und unpassende Paare können eine vom Nutzer konfigurierte Palette unlesbar machen.
Hinweistext bleibt normaler Text; die deaktivierte Textrolle ist dafür nicht gedacht.
Schwarz ist ein gültiger Rückgabewert. Die Unterstützung einer Rolle wird deshalb
über den Systempinsel geprüft, nicht anhand eines RGB-Werts von null.

## Implementierter Umfang

Der Windows-Adapter liest bei aktivem High Contrast acht UI-Rollen: Fenster,
Fenstertext, Schaltfläche, Schaltflächentext, Auswahl, Auswahltext, Link und
Deaktiviert. GetSysColorBrush/GetSysColor werden nur lesend verwendet; Systemfarben
werden nicht geändert und Systempinsel nicht freigegeben. Nicht unterstützte
Rollen erzeugen keine teilweise angewandte Palette.

Die UI übernimmt eine gültige Palette nur bei bekanntem aktivem Systemkontrast.
Eine eigene manuelle Kontrastauswahl ohne Systempalette verwendet weiter den
bisherigen Schwarz-/Weißmodus. Normale Lumen-Farben bleiben erhalten. Fehlende
Systemantworten erhalten den letzten gültigen Stand; ein bekanntes Ausschalten
löscht die Systempalette. Farbrollen werden nicht in die Einstellungsdatei geschrieben.

Fenster-/Kartenflächen, Schrift, funktionale Sternkerne/Kanten und Labeluntergründe
verwenden das Fensterpaar. Eigene Schaltflächen und Icons verwenden das
Schaltflächenpaar, Hover/Pressed und ausgewählte Zeilen das Auswahlpaar. Auch
Editorauswahl, Hinweise, Materialrückfall und gerundete Kanten berücksichtigen die
Palette. Die App stellt Dokumentlinks als eigene Aktionsschaltflächen dar;
sie verwenden deshalb das Schaltflächenpaar. Link-/Deaktiviertrollen sind gelesen,
aber keine Ersatzfarben für gewöhnlichen Hinweistext oder beliebige Icons.

Farbwechsel ohne Flagwechsel aktualisieren Stil und Rastercache. Aktive und noch
inaktive Kartenaufnahmen werden verworfen, damit keine alte Palette überblendet.
Die eigene Auswahl bleibt erhalten und wird nach Ausschalten wieder wirksam.
Die laufende Windows-Abfrage folgt dem vorhandenen Sekundenintervall.

## Tatsächliche lokale Prüfung

Drei ausdrücklich synthetische farbige Paletten, zwei Größen (1000×700 und
1336×840), 100/150 % Schrift: 427 Bedingungen bestehen mit Software und Metal
auf Intel/macOS 14.6.1/23G93, Dichte 2.00. Geprüft sind tatsächliche Zeichenbefehle
und Pixel für Text-/Hintergrundpaare, Hover, Sternkerne, Farbwechsel, aufgehobene
alte Aufnahme, gezielt fehlschlagendes Materialzeichnen und Wiederherstellung,
fehlende Antworten sowie unveränderte eigene Einstellungen. Metal-Aufnahme angesehen.
Die Paletten sind keine auf dem Mac ausgeführten Windows-Kontrastthemen.

1012 normale CPU-Rasterbedingungen und die bisherige Systemdarstellung bestehen.
Vier Material-/Interaktionsprüfungen bestehen (96,35 s); acht weitere aktuelle
Paletten-/Darstellungs-/Layout-/Navigations-/Versionsprüfungen bestehen (109,70 s).
Ein spezieller Windows-Test liest die tatsächlichen acht SDK-Farben und vergleicht
die übertragenen RGB-Kanäle; seine native Ausführung steht noch aus.
427 Bedingungen bestehen auch unter ASan/UBSan: 37 eigene C-Dateien und 61
aktuelle Quell-/Includehashes; externe UI nicht instrumentiert, Leaks aus, kein TSan.
Paket und Installation sind nachfolgend dokumentiert. Kein neuer vollständiger
lokaler 85er-Lauf. Originale: `build/contrast-palette-verification/`.

## Verbleibende Abnahme

Windows-CI muss echten MSVC-Build, SDK-Farbabfrage und Renderer-/Paketwege bestätigen.
Die vier eingebauten Windows-Themen, tatsächliche benutzerdefinierte Farben und
live geänderte Systemeinstellungen benötigen weiterhin eine native und menschliche
Abnahme. Ungeeignete vom Nutzer gewählte Paare werden nicht durch erfundene Farben
ersetzt. Allgemeine Kontrastmessung, Screenreader, weitere Geräte und die übrige
Release-Liste bleiben offen; 1.0 nur nach Nutzerfreigabe.

## Sauberes Intel/macOS-Paket

0.9.69/8d74c8d72d6a, Release: Versionsprüfungen bestehen (0,79 s).
Entpacktes Paket: 126 Desktop-, 167 Tastatur-, 75 Sicherungsbedingungen, zwei
Einstellungsprozesse, CLI-/Runtime-/Lizenz-/Version und Wiederöffnung mit 20
unveränderten Dateien. Echter 68→69→68-Wechsel erhält Dateien/Einstellungen;
Sicherung/Wiederherstellung bestehen, drei App-Aufnahmen angesehen. 98 Dateien
paketgleich installiert; 21 eigene Gedächtnisdateien beim Öffnen unverändert,
installierte Aufnahme angesehen. Normaler Lumen-Look bleibt auf diesem Mac.
Binärminimum macOS 14.0/SDK 15.2; tatsächlich ausgeführt auf Intel/macOS 14.6.1,
keine ausgeführte 14.0-/ARM64-Abnahme.

Archiv-SHA256: `a40be0d9012605df4570ef1e751a577b4c54ab7163f57fee4d9b06c1d6d139f1`.
Rückfallkopie: `/Users/lulus/Projects/SecondBrain/build/previous-dist-0.9.68-20261012-014214`.
Originale: `build/contrast-palette-package-check`, `build/contrast-palette-update`,
`build/contrast-palette-verification/installed.json` und `installed-brain-check.json`.
CI38184984352 zu 8d74c8d: vier Windows-/Linux-Desktopjobs in Arbeit, Mac queued.
Echte SDK-/Windows-Themenabnahme bleibt offen.
