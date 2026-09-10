# agri-solar-poe

[🇯🇵 日本語](README_ja.md) · **English**

M5Stack ATOM PoE Kit + M5Stack ADC Unit V1.1 (ADS1110) + PVSS-03 all-sky pyranometer
→ MQTT + UECS-CCM.
Just a thin layer on top of the
[agri-node-poe-core](https://github.com/yasunorioi/agri-node-poe-core)
library.

## Hardware

- **MCU**: M5Stack ATOM Lite (ESP32-PICO-D4)
- **PoE / Ethernet**: M5Stack ATOM PoE Base (W5500 on SPI)
- **ADC**: M5Stack ADC Unit V1.1 (ADS1110, I²C 0x48)
- **Sensor**: PVSS-03 all-sky pyranometer (0–1 V → 0–1000 W/m² linear)
  - Other voltage-output pyranometers also work with the same ADC. Set the calibration via `wm2_per_volt`

Just connect the ADC Unit to Grove (SDA=G26, SCL=G32), and wire the pyranometer's
output leads to the ADC's differential inputs (V+/V−).

## Configuration (NVS persistence)

`Preferences` namespace `solar-cfg`. Edit from the Web UI at `/config`:

- **Common**: Node ID, hostname, MQTT host/port/user/pass/topic prefix/interval,
  UECS-CCM enable/interval/room/region/priority
- **Sensor-specific**:
  - `wm2_per_volt` — pyranometer calibration constant (1000 for PVSS-03; for others, see the datasheet)
  - `Order (Radiation)` — order of the CCM channel

## Publishing

| Output | Content |
|---|---|
| MQTT `<prefix>` | JSON: `solar_wm2`, `voltage_v`, `sensor_ok`, `node_id`, `uptime_s` |
| CCM `InRadiation.cMC` | Solar radiation (W/m²), PVSS-03 calibration default |

## Build / Flashing

```bash
pio run -e m5atom-poe -t upload                                       # USB-C
pio run -e m5atom-poe -t upload --upload-port agri-solar-01.local     # OTA
```

> 🛠 **Build environment (shared Windows / Linux) and first-time Linux setup (udev, etc.)** →
> [agri-node-poe-core/docs/cross-platform-build.md](https://github.com/yasunorioi/agri-node-poe-core/blob/main/docs/cross-platform-build.md)

## Related projects

- [agri-node-poe-core](https://github.com/yasunorioi/agri-node-poe-core) — common library
- [agri-rain-poe](https://github.com/yasunorioi/agri-rain-poe) — rainfall
- [agri-env-poe](https://github.com/yasunorioi/agri-env-poe) — temperature/humidity + pressure + CO₂
- [agri-flow-poe](https://github.com/yasunorioi/agri-flow-poe) — flow rate
- [OGMS](https://github.com/yasunorioi/OGMS) — main control
- [ccm_rp2350_relay](https://github.com/yasunorioi/ccm_rp2350_relay) — relay
