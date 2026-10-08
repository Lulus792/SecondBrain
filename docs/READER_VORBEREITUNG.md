# Lange Dokumente vorbereiten, ohne die Bewegung anzuhalten

Stand: 8. Oktober 2026. Dies ist der recherchierte Umsetzungs-/Abnahmevertrag,
**noch keine integrierte Produktfunktion**. Installiert bleibt0.9.43. Der
[Reviewbericht](/Users/lulus/Projects/UI_reviewer/reviews/secondbrain/2026-10-08_19-28-45/UI_REVIEW.md) bestätigt SBUI-074 für die kalte Layoutarbeit.

## Belegter Ausgangszustand

Die Hauptschleife baut die gesamte Dokumentansicht, bevor sie zeichnet und
präsentiert. Ein erster langer Wechsel unterbricht damit Kamera und Eingaben.
Drei alternierende Softwarepaare derselben isolierten Projektkopie ergeben
75,32ms kalten Layoutmedian beim aktuellen Code. Ein experimenteller reiner
Breitenmessweg erhielt Glyphen/Zeilen, brachte aber85,14ms Median mit starkem
Ausreißer. Er ist entfernt; sechs Produktdateien entsprechen wieder HEAD.
Die Profilkopie blieb unverändert. Nachweise: `build/cold-layout/paired.json`,
`experiment/sources.json`, `review-proof.json` und `restored-tests.log`.

## Grundlagen vor der neuen Interaktion

Apple [Loading](https://developer.apple.com/design/human-interface-guidelines/loading)
empfiehlt frühe sichtbare Rückmeldung und weitere benutzbare Aktionen während
der Vorbereitung. Apple [Motion](https://developer.apple.com/design/human-interface-guidelines/motion)
empfiehlt kurze, zweckgebundene und unterbrechbare Bewegung. Beide Kapitel am
8. Oktober über offizielle DocC-JSON vollständig gelesen; titelgeprüfte
Originalantworten und SHA-256 im Diagnosebuild erhalten. Spiel-/visionOS-
Spezialhinweise sind keine pauschalen Desktopvorgaben.

Eigene Übertragung: Neue Auswahl/Kamera reagieren früh. Die Inhaltsvorbereitung
hat eigene Zustände und darf weder die gesamte Hauptschleife blockieren noch
einen künstlichen Mindesttimer einführen. Eine Fortschrittszahl aus Dateigröße
oder Absatzzahl wäre keine belastbare Zeitprognose.

## Vorgesehene Zustände

| Zustand | Sichtbarer Inhalt und Verhalten |
| --- | --- |
| bereit | vollständiger letzter gültiger Inhalt und normale Aktionen |
| Vorbereitung angenommen | neue Auswahl/Kamera erkennbar; gewähltes Ziel eindeutig benannt |
| Vorbereitung läuft | ruhige Rückmeldung in der vorhandenen Karte, weitere Navigation/Tastatur benutzbar |
| Ergebnis bereit | Quellen-/Schrift-/Breitenidentität prüfen, vollständigen Plan atomar übernehmen |
| verworfen | spätes Ergebnis bleibt unsichtbar; aktuelle Auswahl/Entwurf bleiben erhalten |
| Fehler | verständliche Rückmeldung, Wiederholen/Abbrechen und Originalinhalt erhalten |

Die Lumen-/Glasgestaltung und festen Kopf-/Aktionsflächen bleiben die Grundlage.
Eine alte Inhaltsvorschau darf keine weiterhin aktiven Links zur falschen
Zielnotiz haben. Der eigentliche Inhaltswechsel und seine Überblendung beginnen
mit einem gültigen fertigen Plan, ohne die Kamerafahrt bis dahin zu sperren.
Kurze bereits vorbereitete Wechsel zeigen keine zusätzliche Ladeansicht.
Bei reduzierter Bewegung bleibt Rückmeldung verständlich, ohne Fahrt/Überblendung.

## Technische Bedingungen

- Ein Job besitzt seine Quellenbytes, Projekt-/Dokumentgeneration, Breite,
  Schrift-/Dichteidentität und anwendbare Darstellungsparameter. Hashwerte
  bestätigen allein keine identische Quelle.
- Neue Notiz, Quelle, Bearbeitung, Schließen, Projektwechsel, Resize und
  Schriftneuladen müssen laufende Arbeit ersetzen oder invalidieren können.
  Bereits eingereihte native Aktionen dürfen kein veraltetes Dokument bedienen.
- Teilpläne verändern keine sichtbaren Zeilenhöhen, Scrollanker oder
  Scrollbargrenzen. Übernommen wird ein vollständiger passender Plan.
- Ein kooperativer Hauptthread-Ansatz muss einzelne sehr große Absätze ebenfalls
  berücksichtigen. Bloß einige Absätze pro Bild zu bearbeiten beweist kein
  Zeitbudget; ein einzelner HarfBuzz-/Bidi-Aufruf kann weiterhin lange dauern.
- Ein Worker benötigt auf seinem eigenen Thread geöffnete/gepflegte Fonts.
  SDL_ttf3.2.2 erlaubt TTF_OpenFont von beliebigen Threads, bindet TTF_CopyFont
  und viele Fontoperationen aber an den Erzeugerthread. Renderer/Textures
  bleiben im UI-Thread. Produkt-UI-Fonts werden nicht unkontrolliert geteilt.
- Fontdateien/-parameter und Glyphenrollen müssen unverändert zur UI-Zuordnung
  passen. Ergebnisse transportieren keine ungeprüften Worker-Fontzeiger in
  die Zeichnung. Schriftneuladen darf keine alten Daten weiterverwenden.
- Speichergrenzen, fehlgeschlagene Allokationen und Lebensdauer aller Jobs
  brauchen eigene Prüfungen. Abbrechen erhält Dateien und ungespeicherte
  Entwürfe; spätes Ergebnis oder Fontfreigabe erzeugen keine Nutzung alter Zeiger.

## Abnahme vor Produktintegration

1. Kurzen/warmen und langen/kalten Wechsel mit derselben Quelle messen;
   Vorbereitungs-, Ereignis-, Zeichnungs- und Presentzeiten getrennt ausweisen.
2. Während der Vorbereitung neue Auswahl, Pfeile, Escape, Schließen, Bearbeiten,
   Projekt-/Quellenwechsel und Größe/Schrift/Dichtewechsel ausführen. Nur der
   letzte gültige Zustand darf erscheinen. Der Entwurfschutz bleibt wirksam.
3. Fertige Glyphen, Bidi, Ligaturen, Stilgrenzen, Zeilenhöhen, Anker und
   Scrollmaxima gegen den vollständigen unabhängigen Referenzplan prüfen.
4. Einzelnen sehr großen Absatz, viele kurze Absätze, Tabellen, leere Absätze,
   Unicode, fehlende Fonts, Speichergrenze und Abbruch während Font-/Jobfreigabe
   gezielt provozieren. Sanitizer-Scope ausdrücklich nennen.
5. macOS/Windows/Linux tatsächlich ausführen; reale Trackpad-/Maus-/Tastatur-/
   Screenreader-Abnahme separat. Keine Framegarantie aus einer einzelnen
   Softwaremessung oder einem Mac-Providerlauf ableiten.

Nächster Umsetzungsschritt: isolierter Vorbereitungsprototyp mit Quellen-/
Fontbesitz und Abbruch/Übernahme, danach Integration in den Dokumentwechsel.
Das Ziel ist ein durchgehend bedienbarer Wechsel einschließlich großer Absätze;
die vollständige Release-Liste und die Sperre für1.0 bleiben bestehen.

## Erster Unterbau

Ab 0.9.44 ist der [Font-/Jobprototyp](FONT_SNAPSHOTS.md) umgesetzt: unveränderliche
Ressourcen, private Worker-Fonts, eigene Textkopie, Abbruch und geprüftes
Übernehmen einer ungebrochenen Zeile. Der produktive Dokumentwechsel benutzt
ihn noch nicht. Vollständiger Umbruch/Styles/Blöcke, begrenzte Jobverwaltung und
sichtbare Integration bleiben der nächste Teil desselben Umsetzungsauftrags.
