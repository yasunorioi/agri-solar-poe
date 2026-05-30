// sensors.h — ADS1110 ADC on Grove I²C, reading a pyranometer voltage.
//
// ADS1110: 16-bit ΔΣ, internal reference ±2.048 V at gain=1. Configured
// here for continuous conversion at 8 SPS / gain=1 / 16-bit (cfg byte 0x0C).
// Each read returns three bytes: high, low, config-readback.

#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

static const int     PIN_SDA = 26;
static const int     PIN_SCL = 32;
static const long    I2C_FREQ_HZ = 100000;
static const uint8_t ADS1110_ADDR             = 0x48;
static const uint8_t ADS1110_CFG_CONT_16BIT   = 0x0C;
static const float   ADS1110_FS_VOLTS         = 2.048f;
static const int16_t ADS1110_FS_COUNTS        = 32767;

extern bool  g_ads_ok;
extern float g_voltage;          // last sampled voltage
extern float g_solar_wm2;        // voltage × wm2_per_volt, clamped

inline bool sensorsBegin() {
  Wire.begin(PIN_SDA, PIN_SCL, I2C_FREQ_HZ);
  Wire.beginTransmission(ADS1110_ADDR);
  Wire.write(ADS1110_CFG_CONT_16BIT);
  uint8_t err = Wire.endTransmission();
  g_ads_ok = (err == 0);
  Serial.printf("[SENS] ADS1110 @ 0x%02X: %s (err=%u)\n",
                ADS1110_ADDR, g_ads_ok ? "OK" : "MISSING", err);
  return g_ads_ok;
}

inline void sensorsPoll() {
  if (!g_ads_ok) {
    // Try to recover (e.g. unit hot-plugged in).
    sensorsBegin();
    return;
  }
  if (Wire.requestFrom((uint8_t)ADS1110_ADDR, (uint8_t)3) != 3) {
    g_ads_ok = false;
    return;
  }
  uint8_t hi  = Wire.read();
  uint8_t lo  = Wire.read();
  uint8_t cfg = Wire.read(); (void)cfg;
  int16_t raw = ((int16_t)hi << 8) | lo;

  g_voltage = (float)raw / (float)ADS1110_FS_COUNTS * ADS1110_FS_VOLTS;

  // Apply user-supplied calibration (default PVSS-03: 1000 W/m² per V).
  float wm2 = g_voltage * (float)g_cfg.wm2_per_volt;
  if (wm2 < 0.0f)     wm2 = 0.0f;
  if (wm2 > 2000.0f)  wm2 = NAN;     // sanity ceiling
  g_solar_wm2 = wm2;
}
