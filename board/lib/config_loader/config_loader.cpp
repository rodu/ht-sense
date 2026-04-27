#include "config_loader.h"
#include <SD.h>
#include <stdio.h>
#include "logger.h"
#include "config_parser.h"

static void logConfig(const AppConfig &cfg)
{
  char buf[Logger::LOG_LINE_MAX];

  snprintf(buf, sizeof(buf), "Config: dataMode=%s", cfg.dataMode);
  Log.info(buf);

  snprintf(buf, sizeof(buf), "Config: logLevel=%s", cfg.logLevel);
  Log.info(buf);

  snprintf(buf, sizeof(buf), "Config: wifiSsid=%s", cfg.wifiSsid[0] ? cfg.wifiSsid : "<not set>");
  Log.info(buf);

  Log.info("Config: wifiPassword=<redacted>");

  snprintf(buf, sizeof(buf), "Config: mqttBroker=%s", cfg.mqttBroker[0] ? cfg.mqttBroker : "<not set>");
  Log.info(buf);

  snprintf(buf, sizeof(buf), "Config: mqttTopic=%s", cfg.mqttTopic[0] ? cfg.mqttTopic : "<not set>");
  Log.info(buf);

  snprintf(buf, sizeof(buf), "Config: syncUrl=%s", cfg.syncUrl[0] ? cfg.syncUrl : "<not set>");
  Log.info(buf);

  Log.info("Config: syncToken=<redacted>");
}

DataMode loadConfig(int sdCsPin)
{
  AppConfig cfg = defaultConfig();

  if (!SD.begin(sdCsPin))
  {
    Log.warn("Config: SD not available, using defaults");
    Log.setLevel(logLevelFromString(cfg.logLevel));
    logConfig(cfg);
    return DataMode::NC;
  }

  File f = SD.open("/config/config.json");
  if (!f)
  {
    Log.warn("Config: config.json not found on SD, using defaults");
    Log.setLevel(logLevelFromString(cfg.logLevel));
    logConfig(cfg);
    return DataMode::NC;
  }

  static char configText[512];
  size_t configLen = f.read(reinterpret_cast<uint8_t *>(configText),
                            sizeof(configText) - 1);
  configText[configLen] = '\0';
  f.close();

  ConfigParser parser;
  if (!parser.parse(configText, configLen, cfg))
    Log.warn("Config: JSON parse error — using defaults");

  Log.setLevel(logLevelFromString(cfg.logLevel));
  logConfig(cfg);

  DataMode mode = parseDataMode(cfg.dataMode);
  return (mode != DataMode::UNKNOWN) ? mode : DataMode::NC;
}
