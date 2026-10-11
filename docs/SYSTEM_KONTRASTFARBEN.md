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
Sauberes Paket und Installation folgen. Kein neuer vollständiger
lokaler 85er-Lauf. Originale: `build/contrast-palette-verification/`.

## Verbleibende Abnahme

Windows-CI muss echten MSVC-Build, SDK-Farbabfrage und Renderer-/Paketwege bestätigen.
Die vier eingebauten Windows-Themen, tatsächliche benutzerdefinierte Farben und
live geänderte Systemeinstellungen benötigen weiterhin eine native und menschliche
Abnahme. Ungeeignete vom Nutzer gewählte Paare werden nicht durch erfundene Farben
ersetzt. Allgemeine Kontrastmessung, Screenreader, weitere Geräte und die übrige
Release-Liste bleiben offen; 1.0 nur nach Nutzerfreigabe.
