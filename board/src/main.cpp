#include <Arduino.h>
#include <SD.h>
#include "logger.h"
#include "data_mode.h"
#include "nc_handler.h"
#include "sensor_reading.h"
#include "config_parser.h"

// SD chip-select pin. Override via build_flags in platformio.ini if needed:
//   build_flags = -DSD_CS_PIN=4
#ifndef SD_CS_PIN
#define SD_CS_PIN 10
#endif

// Active data mode — loaded from config.json on SD card via applyConfig().
static DataMode activeMode = DataMode::NC;
static NcHandler ncHandler;

// Reads /config/config.json from the SD card and parses it into an AppConfig
// struct using the "Default & Override" pattern.  Falls back to safe defaults
// when the SD card or the JSON file is missing or corrupt.
static void applyConfig()
{
  AppConfig cfg = defaultConfig();

  static char configText[512];
  configText[0] = '\0';
  size_t configLen = 0;

  if (!SD.begin(SD_CS_PIN))
  {
    Log.warn("Config: SD not available, using defaults");
    Log.setLevel(logLevelFromString(cfg.logLevel));
    return;
  }

  File f = SD.open("/config/config.json");
  if (!f)
  {
    Log.warn("Config: config.json not found on SD, using defaults");
    Log.setLevel(logLevelFromString(cfg.logLevel));
    return;
  }

  configLen = f.read(reinterpret_cast<uint8_t *>(configText),
                     sizeof(configText) - 1);
  configText[configLen] = '\0';
  f.close();

  // Parse JSON and override defaults with values from the file.
  ConfigParser parser;
  if (!parser.parse(configText, configLen, cfg))
    Log.warn("Config: JSON parse error — using defaults");

  Log.setLevel(logLevelFromString(cfg.logLevel));

  DataMode parsed = parseDataMode(cfg.dataMode);
  if (parsed != DataMode::UNKNOWN)
    activeMode = parsed;
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
  } // wait for USB serial enumeration on Uno R4

  // Wire the global logger to Serial. Default level is INFO; applyConfig()
  // will override it with the LOG_LEVEL value from the .env file on SD.
  Log.setSink([](const char *line)
              { Serial.println(line); });
  Log.setLevel(LogLevel::INFO);

  applyConfig();

  Log.info("HT-Sense starting");
  Log.info(dataModeToString(activeMode));
}

void loop()
{
  // Placeholder sensor reading — replace with real DHT/SHT read once
  // the sensor module is implemented.
  SensorReading reading = {0.0f, 0.0f, 0};

  switch (activeMode)
  {
  case DataMode::NC:
    ncHandler.handle(reading);
    break;

  // Additional mode handlers will be wired here as modules are added.
  default:
    Log.warn("Unhandled data mode");
    break;
  }
}
