# Nachweis der UI-Lizenzbestandteile

Stand: 7. Oktober 2026, ab 0.9.20. Die Inventur betrifft die festgelegten
UI-Abhängigkeiten; sie verändert weder die MIT-Lizenz des eigenen Codes noch
das Dateiformat der Projektgedächtnisse.

## Geprüfter Umfang

Cargo-Metadaten der tatsächlich vorbereiteten macOS-, Windows- und Linux-
Abhängigkeiten sind gegen die unveränderte Lockdatei aus dem durch SHA-256
geprüften AccessKit-C-Quellarchiv abgeglichen. Versionen und Crate-Prüfsummen
bleiben erhalten. Die Vereinigung enthält 113 Komponenten: macOS 20, Windows 24,
Linux 90; gemeinsame Komponenten werden nicht mehrfach gezählt. Build-Crates
sind bewusst eingeschlossen. [Maschinenlesbare Inventur](../third_party/license-manifest.json).

57 unterschiedliche Originaltexte einschließlich Crate-Notices, Autoren und
Unicode-Zusatzlizenz bilden [AccessKit-transitive.txt](../third_party/licenses/AccessKit-transitive.txt).
Bei fehlenden Crate-Lizenzdateien liefern die durch `.cargo_vcs_info.json`
festgelegten Repository-Commits die Originale. Vorhandene Dateien werden gegen
die Crate-Dateiprüfsummen geprüft. `unicode-ident` braucht zusätzlich zur
MIT-/Apache-Alternative die Unicode-Lizenz; sie bleibt enthalten.

[UI-source-notices.txt](../third_party/licenses/UI-source-notices.txt) enthält
zusätzliche unveränderte Copyright-/Lizenzblöcke aus SDL3 und HarfBuzz.
Darunter sind SDLs tatsächlich gebauter YUV-Konverter (BSD-3-Clause), HIDAPI
mit gewählter BSD-Alternative sowie Sun-fdlibm und weitere Autorenhinweise.
Die Sammlung umfasst konservativ auch inaktive Ports und Quellabschnitte.
Sie weist keine unbenutzten Funktionen als Bestandteil des ausgeführten Codes aus.

Die MIT-/Apache-Texte von AccessKit-C und seine Chromium-Hinweise bleiben
vorhanden. Dessen `COPYING.LIB` betrifft laut Original-README ausschließlich
Meson, das unser Cargo-Build nicht verwendet; es ist keine Lizenz des AccessKit-
Laufzeitcodes. [Original](https://github.com/AccessKit/accesskit-c/blob/8b6ed37c20ed4c59390e253407983333053662ba/README.md).

## Anwendung und Pakete

Die zwei Sammlungen sind in der eigenen Lizenzansicht lesbar und kopierbar.
Originale werden nicht als Markdown-Aktionen ausgeführt. App-Ressourcen und
Paketwurzel enthalten dieselben Dateien. Reine Lizenzdateiänderungen lösen
Neu-Verlinken und erneutes Kopieren der Ressourcen aus; sie bleiben nicht nur
in der Paketwurzel aktualisiert. Die Längenmessung für lange Literaltexte
läuft einmal pro Darstellung und nicht erneut für jede Zeile.
Ab 0.9.21 wird zusätzlich das Zeichnen vollständig außerhalb des sichtbaren
Bereichs liegender Literalzeilen übersprungen. Layout und native Textinformationen
werden weiterhin aufgebaut; Originale und Kopierinhalt bleiben unverändert.

Die Ressourcen-, vollständigen Lizenz-/Tastatur- und Paketprüfungen prüfen die
bekannten Texte. `tests/test_ui_license_inventory.py` prüft Vollständigkeit im
festgelegten Umfang, Original-Lock, Ursprung, Dateihashes und die zusätzliche
Unicode-Bedingung; bewusst entfernte/verdoppelte/ersetzte Einträge werden abgewiesen.
Tatsächliche Nachweise und Grenzen stehen in [STATUS](STATUS.md).

## Wiederholen und aktualisieren

`tools/collect_ui_notices.py` ist ein Entwicklerwerkzeug mit Python 3.11 oder
neuer. Anwendung und Pakete benötigen Python nicht. Eingaben sind die drei
mit `cargo metadata --locked --filter-platform ...` erzeugten JSON-Dateien,
das unveränderte geprüfte AccessKit-C-Original sowie die festgelegten SDL3-/
HarfBuzz-Quellbäume. Das Werkzeug benötigt für fehlende Originaltexte Internet.
Es verweigert eine unbekannte Lockdatei oder ungeprüfte Repository-Lizenzquelle.
Die eigentliche Bibliotheksvorbereitung behält ihre vorhandenen Hashprüfungen.

Nach einer Abhängigkeitsänderung werden Metadaten, Originale und Inventur neu
abgeglichen und geprüft; die heutige Sammlung gilt nicht automatisch für
andere Versionsstände oder eine beliebige vorhandene SDL-Installation.
[Aktualisierungsablauf](ABHAENGIGKEITEN_WARTUNG.md).

## Noch offen

Die eingebettete Rust-Standardbibliothek/Compilerlaufzeit und Systemruntimes
brauchen einen zusätzlichen Abgleich. In der macOS-AccessKit-Bibliothek ist
Compiler-Commit `48a229ceaefd4985c50990b14116b6d856af0985` beobachtet, dessen
Original `src/version` 1.98.1 angibt. Der lokale Rust-Compiler ist 1.99.0.
Diese Zuordnung allein belegt noch nicht sämtliche Laufzeit-Unterbestandteile.
Die vollständige transitive Release-Abnahme bleibt deshalb offen.
Der [Rust-Laufzeitnachweis](RUST_RUNTIME_NACHWEIS.md) konkretisiert die zwei
beobachteten Compilerstände, den zusätzlichen Library-Lockumfang und die
Buildaufzeichnung für Windows/Linux.

Dynamische Systembibliotheken sind von tatsächlich mitgelieferten Kopien zu
unterscheiden. GLib/GIO/D-Bus werden im Linux-Paket nicht mitkopiert. Ihre
Versionen, Systemhinweise und die übrigen Toolchain-Anteile bleiben Gegenstand
des abschließenden Abgleichs. Für den bisherigen Schritt wird keine vollständige
Lizenzfreigabe aller möglichen Builds behauptet.

## Konsolidierter Abgleich vom 12. Oktober

Die früheren offenen Punkte oben sind anhand der späteren Originalnachweise
einzuordnen. Für die tatsächlich geprüften Entwicklungsstände gilt:

| Bestandteil | Belegter Umfang |
| --- | --- |
| Eigener C-Code | MIT auf Nutzerentscheidung; [LICENSE](../LICENSE) |
| UI-Bibliotheken und Schriften | Festgelegte Quellen,113 Cargo-Komponenten und Originaltexte im vorhandenen Inventurumfang |
| Rust-Standardbibliothek/Compilerlaufzeit | Konservative Quellenabdeckung der beobachteten Compilercommits und Ziele; native Compiler-/Lock-/Lizenzabgleiche in [RUST_RUNTIME_NACHWEIS.md](RUST_RUNTIME_NACHWEIS.md) |
| Linux-Startobjekte | Tatsächliche glibc-/GCC-Paketzuordnung, Copyrighttexte und besonderer Quellregelabgleich in [LINKER_BESTAND.md](LINKER_BESTAND.md) |
| Windows-CRT in App/CLI und AccessKit-DLL | Native Karten und SDK-Dateihashes; DLL-Kopien bytegenau den Release-SDK-Dateien zugeordnet, Debug und Release getrennt |
| Dynamische Systemkomponenten | [Paket- und Laufzeitnachweise](PAKET_LAUFZEIT.md), einschließlich beobachteter normaler Starts; Systemdateien sind von mitgelieferten Kopien zu unterscheiden |

Der Nachweis bestimmt konkrete Inhalte und Quellen. Er ordnet nicht jede
optimierte Rust-In-tree-Funktion unabhängig ihrem ursprünglichen Quellabschnitt
zu und ersetzt keine abschließende Verteilungsbedingungenprüfung.

### Microsoft-Verteilungsbedingungen

Am12.Oktober wurden die offiziellen [Visual-Studio-2026-Bedingungen](https://visualstudio.microsoft.com/license-terms/vs2026-ga-pro-enterprise/)
und die [2026-Verteilungsliste](https://learn.microsoft.com/en-us/visualstudio/releases/2026/redistribution)
erneut geprüft. Die Liste setzt eine gültige Lizenz der betreffenden Software
und deren Bedingungen voraus. Der Abschnitt „Distributable Code“ im Original
bindet die Verteilung unter anderem an eigene wesentliche Funktionalität und
schützende Bedingungen für Distributoren und Endnutzer; er enthält außerdem
Haftungs-/Freistellungsbedingungen und Beschränkungen für unveröffentlichte
Komponenten. Das sind Microsoft-Bedingungen, keine neue Lizenz des eigenen C-Codes.

Die [allgemeinen Visual-C++-Verteilungshinweise](https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files?view=msvc-170)
verweisen ebenfalls auf passende Softwarebedingungen. Die tatsächlichen
SecondBrain-Pakete enthalten statisch eingebaute CRT-Anteile und keinen separat
gebündelten vc_redist-Installer. Eine Liste für separat verteilte Dateien wird
deshalb nicht pauschal als vollständige Freigabe dieser Einbettung ausgegeben.
SDK-Dateihashes und ein vorinstallierter CI-Compiler belegen für sich allein
keinen individuellen Lizenzvertrag des späteren Herausgebers.

Die konkrete rechtliche Zuordnung der eingebetteten SDK-Anteile und die
endgültigen Veröffentlichungsbedingungen bleiben vor1.0 offen. Es wurde weder
ein Vertrag für den Nutzer angenommen noch eine neue Endnutzervereinbarung
eingeführt. Geprüfte Originaldokumente und Hashes stehen unter
`build/link-map-runtime-review/`; technische Originale unter
`build/accesskit-map-native/`. Die [Release-Abnahme](RELEASE_ABNAHME.md) bleibt maßgeblich.
