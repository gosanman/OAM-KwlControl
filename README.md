# OAM-KwlControl — KNX-Lüftersteuerung 0–10 V (KNXFANDRV)

OpenKNX-Applikation für dezentrale Pendellüfter mit 0–10-V-Stellsignal. Ein RP2040
steuert über einen galvanisch getrennten I²C-Bus zwei GP8413-DACs an, die vier
Stellsignale S1…S4 erzeugen. Entwickelt an e²60, ego (zwei Motoren, belegt zwei
Kanäle) und RA 15-60.

Dieses Repository ist die **Geräteapplikation**: Firmware-Rahmen, ETS-Produkt,
Board-Header, Build- und Release-Skripte, Simulator und Projektdokumentation. Die
eigentliche Lüftungslogik steckt im Funktionsmodul
[OFM-KwlControl](https://github.com/gosanman/OFM-KwlControl), das unter
`lib/OFM-KwlControl` eingebunden wird.

Applikationsversion: **0.6**

> **5,00 V ist Stillstand. 0 V ist Volllast.** Das Stellsignal ist bipolar um 5 V
> aufgebaut. Die DACs halten keinen Einschaltwert: Nach dem Einschalten der 12 V
> liegen 0 V an, bis die Firmware den sicheren Zustand schreibt — die Lüfter laufen
> kurz mit voller Drehzahl. **Liegen 12 V ohne KNX-Bus an, bleibt es dabei.** Die
> Einzelheiten stehen in der
> [Applikationsbeschreibung](https://github.com/gosanman/OFM-KwlControl/blob/v1/doc/Applikationsbeschreibung-Kwl.md#bevor-sie-anschließen).

## Funktionen

- **Drei Ebenen:** bis 8 Räume (Sensoren, Betriebsarten, Führungen), bis 8 Verbünde
  (Stufenregel, Takt, Wärmerückgewinnung durch Pendeln) und bis 12 Lüfter (Typ,
  Kennlinie, Stellkanal, Leitungskompensation). Sichtbar ist, was die gewählte
  Platine hergibt.
- **Stufen 0…4** aus Handbetrieb, Automatik und Betriebsarten, mit fester Vorfahrt:
  Sperre → Schutz → Zu-/Abluftanforderung → Verbund → Hand → Automatik.
- **Führungen** über Feuchte, CO₂, VOC und Temperatur, mit Feuchtevergleich innen/außen,
  Frost- und Hitzeschutz.
- **Sieben Betriebsarten** (Komfort, Standby, Nacht, Frost-/Gebäudeschutz,
  Stoßlüften, Temperatur-Absenkung, Ruhe), mit Zwangsobjekten, Zwangsbetriebsart
  und Rückmeldung für Bedienfelder.
- **Richtungswechsel** immer über 5,00 V und eine parametrierbare Totzeit.
- **Diagnose** über die serielle Konsole oder das Diagnose-Objekt (`kwl st`,
  `kwl r1`, `kwl f1`, `kwl grp`), Fehlercodes und Alarm je Lüfter.
- Dazu die OpenKNX-Logikkanäle (OFM-LogicModule), Konfigurationstransfer und
  Dateiübertragung.

## Hardware

| | |
|---|---|
| Platine | KNXFANDRV Rev 0.1, eigene Platine mit OpenKNX-BCU-Modul GN100 |
| Prozessor | RP2040, 16 MiB Flash |
| Ausgänge | 2 × GP8413 (U2 = 0x58, U3 = 0x59) hinter ADuM1250, I²C auf GPIO2/GPIO3 |
| Kanäle | S1 = U2.VOUT0, S2 = U2.VOUT1, S3 = U3.VOUT0, S4 = U3.VOUT1 |
| Versorgung Ausgänge | 12 V, getrennt vom KNX-Bus |

Alle Pins, Adressen und das gemessene Busformat der DACs stehen im Board-Header
[include/KnxFanDrv_Rev01.h](include/KnxFanDrv_Rev01.h).

## Bauen

### Voraussetzungen

- [PlatformIO](https://platformio.org/) (VS Code oder Kommandozeile)
- [OpenKNXproducer](https://github.com/OpenKNX/OpenKNXproducer) unter `~/bin/`,
  für die knxprod zusätzlich eine installierte ETS
- Git mit Recht auf symbolische Links (Windows: Entwicklermodus oder Administrator)
- Für den Simulator: MinGW-GCC und Python 3

### Erstes Klonen

```
git clone https://github.com/gosanman/OAM-KwlControl
cd OAM-KwlControl
restore/Restore-Dependencies.ps1
```

Das Skript klont die Bibliotheken auf die Stände aus
[dependencies.txt](dependencies.txt) neben das Projekt und verlinkt sie unter `lib/`:

| Bibliothek | Zweck |
|---|---|
| `OFM-KwlControl` | Lüftungslogik (Räume, Verbünde, Lüfter, DAC-Treiber) |
| `OGM-Common` | OpenKNX-Rahmen: Module, Konsole, Flash, LEDs |
| `knx` | KNX-Stack |
| `OFM-LogicModule` | Logikkanäle |
| `OFM-ConfigTransfer` | ETS-Konfiguration exportieren und importieren |
| `OFM-FileTransferModule` | Dateisystem über den Bus |

### Firmware und ETS-Produkt

```
OpenKNXproducer create --Debug -h include/knxprod.h src/Kwl     knxprod und knxprod.h
pio run -e develop_KnxFanDrv                                    Entwicklungsfirmware
scripts/Build-Release.ps1 Release                               Release-Paket
scripts/Build-Release.ps1                                       Beta-Paket
```

Das Release-Paket landet als `release/KwlControl-Release-<Version>.zip` mit
knxprod, Firmware (.uf2) und Upload-Skripten. Alle Schritte gibt es auch als
VS-Code-Tasks (*Terminal → Task ausführen*).

| Umgebung | Inhalt |
|---|---|
| `develop_KnxFanDrv` | Firmware mit Debug-Ausgabe und Heartbeat |
| `release_KnxFanDrv` | Firmware mit Watchdog, ohne Debug |
| `test_dac` | Messfirmware für den Hardwarenachweis, ohne KNX und ohne knxprod |

### Tests und Simulator

```
cd ../OFM-KwlControl && pio test -e native                      Logiktests, ohne Hardware
powershell -File sim/build.ps1                                  Simulator bauen
python sim/kwlsim.py                                            Simulator starten (Port 8765)
python sim/selftest.py                                          Simulator-Selbsttest
```

Der Simulator übersetzt den unveränderten Firmware-Code für den PC und zeigt im
Browser die Parameter in ETS-Ansicht, alle Objekte und die Ausgänge S1…S4. Bedienung
und Grenzen: [sim/README.md](sim/README.md).

## Dokumentation

| Dokument | Inhalt |
|---|---|
| [Applikationsbeschreibung](https://github.com/gosanman/OFM-KwlControl/blob/v1/doc/Applikationsbeschreibung-Kwl.md) | Bedienung, Parameterseiten, Kommunikationsobjekte, Fehlercodes, Konsole (im OFM-Repo) |
| [Ablaufdiagramme](https://github.com/gosanman/OFM-KwlControl/blob/v1/doc/technical/Ablaufdiagramme.md) | Abläufe in der Firmware: Start, Hauptschleife, Vorfahrt, Takt, Ausgabe (im OFM-Repo) |
| [sim/README.md](sim/README.md) | Simulator |

## Lizenz

GPL-3.0, siehe [LICENSE](LICENSE).
