#include "config_parser.h"
#include <ArduinoJson.h>
#include <string.h>

// ---------------------------------------------------------------------------
// defaultConfig
// ---------------------------------------------------------------------------

AppConfig defaultConfig()
{
  AppConfig cfg = {}; // zero-initialise every char array field

  strncpy(cfg.dataMode, "NC", sizeof(cfg.dataMode) - 1);
  strncpy(cfg.logLevel, "INFO", sizeof(cfg.logLevel) - 1);
  // wifiSsid, wifiPassword, mqttBroker, syncUrl, syncToken left as ""
  strncpy(cfg.mqttTopic, "ht-sense/data", sizeof(cfg.mqttTopic) - 1);

  return cfg;
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

// ---------------------------------------------------------------------------
// ConfigParser::crc32  (ISO 3309 / PKZIP CRC32, polynomial 0xEDB88320)
// ---------------------------------------------------------------------------

uint32_t ConfigParser::crc32(const uint8_t *data, size_t len)
{
  uint32_t crc = 0xFFFFFFFFu;

  for (size_t i = 0; i < len; i++)
  {
    crc ^= data[i];
    for (int bit = 0; bit < 8; bit++)
    {
      if (crc & 1u)
        crc = (crc >> 1) ^ 0xEDB88320u;
      else
        crc >>= 1;
    }
  }

  return ~crc;
}
