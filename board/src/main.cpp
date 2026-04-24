#include <Arduino.h>
#include "logger.h"
#include "data_mode.h"
#include "nc_handler.h"
#include "sensor_reading.h"

// Active data mode — will be read from .env on SD card once config_parser exists.
static DataMode activeMode = DataMode::NC;
static NcHandler ncHandler;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
  } // wait for USB serial enumeration on Uno R4

  // Wire the global logger to Serial for this session.
  Log.setSink([](const char *line)
              { Serial.println(line); });
  Log.setLevel(LogLevel::INFO);

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
