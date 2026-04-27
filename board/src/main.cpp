#include <Arduino.h>
#include "logger.h"
#include "data_mode.h"
#include "nc_handler.h"
#include "sensor_reading.h"
#include "config_loader.h"

// SD chip-select pin. Override via build_flags in platformio.ini if needed:
//   build_flags = -DSD_CS_PIN=4
#ifndef SD_CS_PIN
#define SD_CS_PIN 10
#endif

static DataMode activeMode = DataMode::NC;
static NcHandler ncHandler;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
  } // wait for USB serial enumeration on Uno R4

  // Wire the global logger to Serial. Default level is INFO; loadConfig()
  // will override it with the LOG_LEVEL value from the config file on SD.
  Log.setSink([](const char *line)
              { Serial.println(line); });
  // temporary default — overridden by LOG_LEVEL from config
  Log.setLevel(LogLevel::INFO);

  activeMode = loadConfig(SD_CS_PIN);

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
