#include "config_loader.h"
#include <SD.h>
#include "logger.h"
#include "config_parser.h"

DataMode loadConfig(int sdCsPin)
{
  AppConfig cfg = defaultConfig();

  if (!SD.begin(sdCsPin))
  {
    Log.warn("Config: SD not available, using defaults");
    Log.setLevel(logLevelFromString(cfg.logLevel));
    return DataMode::NC;
  }

  File f = SD.open("/config/config.json");
  if (!f)
  {
    Log.warn("Config: config.json not found on SD, using defaults");
    Log.setLevel(logLevelFromString(cfg.logLevel));
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

  DataMode mode = parseDataMode(cfg.dataMode);
  return (mode != DataMode::UNKNOWN) ? mode : DataMode::NC;
}
