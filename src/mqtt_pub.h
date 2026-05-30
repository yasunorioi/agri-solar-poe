// mqtt_pub.h — solar JSON payload to the configured topic prefix.

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AgriNode.h>
#include "config.h"
#include "sensors.h"

inline bool mqttPublishSolar() {
  if (!agri::MQTT::hasHost(g_cfg.common) || !agri::MQTT::connected()) return false;

  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  root["node_id"]   = g_cfg.common.node_id;
  root["uptime_s"]  = millis() / 1000;
  root["sensor_ok"] = g_ads_ok;
  if (g_ads_ok) {
    root["voltage_v"]  = g_voltage;
    root["solar_wm2"]  = g_solar_wm2;
  }

  char payload[224];
  size_t n = serializeJson(doc, payload, sizeof(payload));
  bool ok = agri::MQTT::mqtt.publish(g_cfg.common.mqtt_topic_prefix,
                                     (const uint8_t*)payload, n, true);
  Serial.printf("[MQTT] %s %s (%u bytes)\n",
                g_cfg.common.mqtt_topic_prefix, ok ? "OK" : "FAIL",
                (unsigned)n);
  return ok;
}
