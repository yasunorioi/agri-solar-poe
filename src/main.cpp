// agri-solar-poe — M5 ATOM PoE solar radiation node.
// M5Stack ADC Unit V1.1 (ADS1110) + pyranometer (PVSS-03 by default).

#include <Arduino.h>
#include <Wire.h>
#include <AgriNode.h>

#include "config.h"
#include "sensors.h"
#include "mqtt_pub.h"
#include "ccm_pub.h"

const char *FW_NAME    = "agri-solar-poe";
const char *FW_VERSION = "0.1.0";

AppConfig g_cfg;

bool  g_ads_ok      = false;
float g_voltage     = 0.0f;
float g_solar_wm2   = NAN;

static String renderDashboardSensors() {
  String s; s.reserve(280);
  char buf[12];
  s = F("<h3>Solar radiation</h3><table>");
  if (g_ads_ok) {
    dtostrf(isnan(g_solar_wm2) ? 0.0f : g_solar_wm2, 1, 1, buf);
    s += "<tr><th>Irradiance</th><td>"; s += buf; s += " W/m²</td></tr>";
    dtostrf(g_voltage, 1, 4, buf);
    s += "<tr><th>ADC voltage</th><td>"; s += buf; s += " V</td></tr>";
    s += "<tr><th>Calibration</th><td>"; s += g_cfg.wm2_per_volt; s += " W/m² per V</td></tr>";
  } else {
    s += "<tr><th>ADS1110</th><td>NOT detected</td></tr>";
  }
  s += F("</table>");
  return s;
}

static String renderConfigSensorRows() {
  String s;
  auto row = [&](const char *label, const String &input) {
    s += "<tr><th>"; s += label; s += "</th><td>"; s += input; s += "</td></tr>";
  };
  row("W/m² per V (calibration)",
      "<input type=number name=wm2v value='" + String(g_cfg.wm2_per_volt) + "'>");
  row("Order (Radiation)",
      "<input type=number name=ccm_ord value='" + String(g_cfg.ccm_order) + "'>");
  return s;
}

static void applyConfigSensorForm(const String &body) {
  g_cfg.wm2_per_volt = (uint16_t)agri::parseFormInt(body, "wm2v",    g_cfg.wm2_per_volt);
  g_cfg.ccm_order    = (int16_t) agri::parseFormInt(body, "ccm_ord", g_cfg.ccm_order);
}

static void addStatusFields(JsonObject doc) {
  doc["sensor_ok"] = g_ads_ok;
  if (g_ads_ok) {
    doc["voltage_v"] = g_voltage;
    doc["solar_wm2"] = isnan(g_solar_wm2) ? 0.0f : g_solar_wm2;
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.printf("\n=== %s v%s ===\n", FW_NAME, FW_VERSION);

  agri::Led::begin();
  loadConfig();
  Serial.printf("[CFG] node=%s mqtt_host=%s ccm=%s wm2/V=%u\n",
                g_cfg.common.node_id,
                g_cfg.common.mqtt_host[0] ? g_cfg.common.mqtt_host : "(unset)",
                g_cfg.common.ccm_enabled ? "on" : "off",
                g_cfg.wm2_per_volt);

  sensorsBegin();

  agri::Network::begin(g_cfg.common.hostname);
  agri::Network::waitForLease();

  agri::ccmBegin();
  agri::MQTT::begin();

  agri::WebHooks hooks;
  hooks.nodeTitle             = [](){ return FW_NAME; };
  hooks.renderDashboardSensors= renderDashboardSensors;
  hooks.renderConfigSensorRows= renderConfigSensorRows;
  hooks.applyConfigSensorForm = applyConfigSensorForm;
  hooks.addStatusFields       = addStatusFields;
  hooks.saveConfig            = [](){ saveConfig(); };
  agri::WebUI::begin(g_cfg.common, hooks, FW_NAME, FW_VERSION);

  agri::mdnsBegin(g_cfg.common.hostname);
  agri::otaBegin(g_cfg.common.hostname);

  Serial.println("[BOOT] ready");
}

void loop() {
  agri::otaHandle();
  agri::WebUI::handle(agri::Network::link_up, agri::Network::have_lease);

  uint32_t now = millis();

  static uint32_t lastSensorPoll = 0;
  if (now - lastSensorPoll >= 1000) {
    lastSensorPoll = now;
    sensorsPoll();
  }

  if (agri::networkUp() && agri::MQTT::hasHost(g_cfg.common)) {
    if (!agri::MQTT::connected()) {
      static uint32_t lastTry = 0;
      if (now - lastTry > 5000) { lastTry = now; agri::MQTT::reconnect(g_cfg.common); }
    } else {
      agri::MQTT::loop();
      static uint32_t lastPub = 0;
      uint32_t interval = (uint32_t)g_cfg.common.mqtt_interval_s * 1000UL;
      if (now - lastPub >= interval) {
        lastPub = now;
        if (mqttPublishSolar()) agri::Led::flashPublish();
      }
    }
  }

  if (agri::networkUp() && g_cfg.common.ccm_enabled) {
    static uint32_t lastCcm = 0;
    uint32_t interval = (uint32_t)g_cfg.common.ccm_interval_s * 1000UL;
    if (now - lastCcm >= interval) {
      lastCcm = now;
      if (ccmPublish()) agri::Led::flashPublish();
    }
  }

  agri::LedState desired;
  if (!agri::networkUp())                                                desired = agri::LED_NO_LINK;
  else if (!g_ads_ok)                                                    desired = agri::LED_NO_SENSOR;
  else if (agri::MQTT::hasHost(g_cfg.common) && !agri::MQTT::connected()) desired = agri::LED_NO_MQTT;
  else                                                                   desired = agri::LED_OK;
  agri::Led::set(desired);

  static uint32_t lastStatus = 0;
  if (now - lastStatus >= 30000) {
    lastStatus = now;
    Serial.printf("[STATUS] link=%d lease=%d mqtt=%d ads=%d V=%.4f wm2=%.1f up=%lus\n",
                  agri::Network::link_up, agri::Network::have_lease,
                  agri::MQTT::connected(), g_ads_ok, g_voltage,
                  isnan(g_solar_wm2) ? 0.0f : g_solar_wm2,
                  (unsigned long)(now / 1000));
  }

  delay(20);
}
