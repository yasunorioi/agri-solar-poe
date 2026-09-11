# agri-solar-poe

**日本語** · [🇬🇧 English](README_en.md)

[M5Stack ATOM PoE Kit](https://docs.m5stack.com/en/atom/atom_poe) + [M5Stack ADC Unit V1.1 (ADS1110)](https://docs.m5stack.com/en/unit/Unit-ADC_V1.1) + [PVSS-03](https://www.sanko-web.co.jp/product/pvssap/product-pvss-03/) 全天日射計
→ MQTT + UECS-CCM。
[agri-node-poe-core](https://github.com/yasunorioi/agri-node-poe-core)
ライブラリ上に薄く乗っているだけ。

## ハードウェア

- **MCU**: [M5Stack ATOM Lite](https://docs.m5stack.com/en/core/ATOM%20Lite) (ESP32-PICO-D4)
- **PoE / Ethernet**: [M5Stack ATOM PoE Base](https://docs.m5stack.com/en/atom/Atomic%20PoE%20Base) (W5500 on SPI)
- **ADC**: M5Stack ADC Unit V1.1 (ADS1110, I²C 0x48)
- **センサー**: PVSS-03 全天日射計 (0–1 V → 0–1000 W/m² リニア)
  - 他の電圧出力型 pyranometer も同 ADC で OK。calibration を `wm2_per_volt` で設定

Grove (SDA=G26, SCL=G32) に ADC Unit を繋ぎ、ADC の差動入力 (V+/V−) に
日射計の出力線をつなぐだけ。

## 設定 (NVS 永続化)

`Preferences` ネームスペース `solar-cfg`。Web UI の `/config` から編集:

- **共通**: Node ID, hostname, MQTT host/port/user/pass/topic prefix/interval,
  UECS-CCM enable/interval/room/region/priority
- **センサー固有**:
  - `wm2_per_volt` — pyranometer の校正定数 (PVSS-03 で 1000、他は仕様書参照)
  - `Order (Radiation)` — CCM チャネルの order

## 配信

| 出力 | 内容 |
|---|---|
| MQTT `<prefix>` | JSON: `solar_wm2`, `voltage_v`, `sensor_ok`, `node_id`, `uptime_s` |
| CCM `InRadiation.cMC` | 日射量 (W/m²)、PVSS-03 calibration デフォルト |

## ビルド / 焼き込み

```bash
pio run -e m5atom-poe -t upload                                       # USB-C
pio run -e m5atom-poe -t upload --upload-port agri-solar-01.local     # OTA
```

> 🛠 **ビルド環境（Windows / Linux 共用）・Linux 初回セットアップ（udev 等）** →
> [agri-node-poe-core/docs/cross-platform-build.md](https://github.com/yasunorioi/agri-node-poe-core/blob/main/docs/cross-platform-build.md)

## 関連プロジェクト

- [agri-node-poe-core](https://github.com/yasunorioi/agri-node-poe-core) — 共通ライブラリ
- [agri-rain-poe](https://github.com/yasunorioi/agri-rain-poe) — 雨量
- [agri-env-poe](https://github.com/yasunorioi/agri-env-poe) — 温湿度 + 気圧 + CO₂
- [agri-flow-poe](https://github.com/yasunorioi/agri-flow-poe) — 流量
- [OGMS](https://github.com/yasunorioi/OGMS) — メイン制御
- [ccm_rp2350_relay](https://github.com/yasunorioi/ccm_rp2350_relay) — リレー
