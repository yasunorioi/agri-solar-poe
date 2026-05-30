// config.h — agri-solar-poe NVS-backed config.
//
// One CCM channel (InRadiation.cMC). The sensor calibration scale lets
// the same firmware work with sensors that aren't PVSS-03 — set it to
// (W/m² per volt) for your pyranometer (PVSS-03 is 1000 W/m²/V).

#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <AgriCommonConfig.h>

struct AppConfig {
  agri::CommonConfig common;
  uint16_t           wm2_per_volt;   // sensor calibration (default 1000 = PVSS-03)
  int16_t            ccm_order;      // InRadiation.cMC
};

extern AppConfig g_cfg;

inline void setDefaults() {
  agri::commonDefaults(g_cfg.common,
                       "solar_node_01", "agri-solar-01",
                       "agriha/h01/sensor/Solar",
                       /*default_ccm_region=*/11);
  g_cfg.wm2_per_volt = 1000;
  g_cfg.ccm_order    = 1;
}

inline void loadConfig() {
  setDefaults();
  Preferences p;
  if (!p.begin("solar-cfg", true)) return;
  agri::commonLoad(g_cfg.common, p);
  g_cfg.wm2_per_volt = p.getUShort("wm2v",    g_cfg.wm2_per_volt);
  g_cfg.ccm_order    = p.getShort ("ccm_ord", g_cfg.ccm_order);
  p.end();
}

inline bool saveConfig() {
  Preferences p;
  if (!p.begin("solar-cfg", false)) return false;
  agri::commonSave(g_cfg.common, p);
  p.putUShort("wm2v",    g_cfg.wm2_per_volt);
  p.putShort ("ccm_ord", g_cfg.ccm_order);
  p.end();
  return true;
}
