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

// Active data mode — loaded from .env on SD card via applyConfig().
static DataMode activeMode = DataMode::NC;
static NcHandler ncHandler;

// Reads /.env (falling back to /config/.env) from the SD card and applies
// LOG_LEVEL and DATA_MODE. Falls back to safe defaults when the SD card or
// the file is not available — allowing the firmware to run without SD during
// NC mode development and testing.
static void applyConfig()
{
  static char configText[512];
  configText[0] = '\0';

  if (SD.begin(SD_CS_PIN))
  {
    File f = SD.open("/.env");
    if (!f)
      f = SD.open("/config/.env");

    if (f)
    {
      size_t len = f.read(reinterpret_cast<uint8_t *>(configText),
                          sizeof(configText) - 1);
      configText[len] = '\0';
      f.close();
    }
    else
    {
      Log.warn("Config: .env not found on SD, using defaults");
    }
  }
  else
  {
    Log.warn("Config: SD not available, using defaults");
  }

  ConfigParser parser;
  char valueBuf[32];

  valueBuf[0] = '\0';
  if (parser.findValue(configText, "LOG_LEVEL", valueBuf, sizeof(valueBuf)))
    Log.setLevel(logLevelFromString(valueBuf));

  valueBuf[0] = '\0';
  if (parser.findValue(configText, "DATA_MODE", valueBuf, sizeof(valueBuf)))
  {
    DataMode parsed = parseDataMode(valueBuf);
    if (parsed != DataMode::UNKNOWN)
      activeMode = parsed;
  }
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
