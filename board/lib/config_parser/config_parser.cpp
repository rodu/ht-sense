#include "config_parser.h"
#include <ArduinoJson.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Default configuration values — edit here only.
// constexpr allows the compiler to place this in read-only flash (ROM),
// keeping it out of precious RAM on embedded targets.
// ---------------------------------------------------------------------------

namespace
{
  constexpr AppConfig kDefaultConfig = {
      "NC",            // dataMode      — NC | LS | SS | RT | LSRT | SSRT
      "INFO",          // logLevel      — DEBUG | INFO | WARN | ERROR | NONE
      "",              // wifiSsid      — any string (network SSID)
      "",              // wifiPassword  — any string (sensitive — never log)
      "",              // mqttBroker    — hostname or IP address
      "ht-sense/data", // mqttTopic     — any valid MQTT topic string
      "",              // syncUrl       — any valid HTTP/HTTPS URL
      "",              // syncToken     — any string (sensitive — never log)
  };
} // namespace

AppConfig defaultConfig()
{
  return kDefaultConfig;
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static void safeCopy(const char *src, char *dst, size_t dstLen)
{
  if (!src || !dst || dstLen == 0)
    return;
  strncpy(dst, src, dstLen - 1);
  dst[dstLen - 1] = '\0';
}

// ---------------------------------------------------------------------------
// ConfigParser::parse
// ---------------------------------------------------------------------------

bool ConfigParser::parse(const char *jsonText, size_t len, AppConfig &cfg) const
{
  if (!jsonText || len == 0)
    return false;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, jsonText, len);
  if (err)
    return false;

  bool found = false;

  // Helper: apply a string field from the JSON variant to a char buffer.
  auto apply = [&](const char *src, char *dst, size_t dstLen)
  {
    if (src)
    {
      safeCopy(src, dst, dstLen);
      found = true;
    }
  };

  // Top-level scalar fields
  apply(doc["dataMode"] | (const char *)nullptr, cfg.dataMode, sizeof(cfg.dataMode));
  apply(doc["logLevel"] | (const char *)nullptr, cfg.logLevel, sizeof(cfg.logLevel));

  // Nested: wifi
  apply(doc["wifi"]["ssid"] | (const char *)nullptr, cfg.wifiSsid, sizeof(cfg.wifiSsid));
  apply(doc["wifi"]["password"] | (const char *)nullptr, cfg.wifiPassword, sizeof(cfg.wifiPassword));

  // Nested: mqtt
  apply(doc["mqtt"]["broker"] | (const char *)nullptr, cfg.mqttBroker, sizeof(cfg.mqttBroker));
  apply(doc["mqtt"]["topic"] | (const char *)nullptr, cfg.mqttTopic, sizeof(cfg.mqttTopic));

  // Nested: sync
  apply(doc["sync"]["url"] | (const char *)nullptr, cfg.syncUrl, sizeof(cfg.syncUrl));
  apply(doc["sync"]["token"] | (const char *)nullptr, cfg.syncToken, sizeof(cfg.syncToken));

  return found;
}
