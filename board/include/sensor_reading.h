#pragma once
#include <stdint.h>

/**
 * A single humidity and temperature measurement produced by the HT sensor.
 *
 * timestamp is seconds since the Unix epoch (UTC) when the reading was taken.
 * Use uint32_t throughout to keep the type portable across Arduino and native
 * toolchains (avoids relying on Arduino's unsigned long definition).
 */
struct SensorReading
{
  float temperature;  // Degrees Celsius
  float humidity;     // Percent relative humidity (0.0 – 100.0)
  uint32_t timestamp; // Seconds since Unix epoch (UTC)
};
