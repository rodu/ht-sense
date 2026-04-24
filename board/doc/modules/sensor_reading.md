# Module: sensor_reading

**Location:** `lib/sensor_reading/`
**Files:** `sensor_reading.h` (header-only)
**Environment:** native + uno_r4_wifi (no Arduino dependency)
**Tests:** none — the struct has no logic; it is validated indirectly by all
modules that consume it.

---

## Purpose

A single shared data type that represents one measurement taken by the humidity
and temperature sensor. Having one canonical struct prevents multiple modules
from defining their own incompatible representations.

---

## Definition

```cpp
struct SensorReading {
    float    temperature;  // Degrees Celsius
    float    humidity;     // Percent relative humidity (0.0 – 100.0)
    uint32_t timestamp;    // Seconds since Unix epoch (UTC)
};
```

---

## Design Notes

- **Header-only.** There is no `.cpp` file — the struct has no methods or
  static members, so no translation unit is needed.
- **`uint32_t` timestamp.** Using `uint32_t` (from `<stdint.h>`) rather than
  `unsigned long` keeps the type portable across Arduino toolchains and the
  native test environment. It represents seconds since the Unix epoch, which
  overflows in 2106 — acceptable for this hobbyist project.
- **No validation in the struct.** Range checks (e.g. humidity 0–100, plausible
  temperature) are the responsibility of the sensor driver that produces the
  reading, not this data container.
- **No Arduino dependency.** The only include is `<stdint.h>`, so this struct
  can be used freely in native host tests.

---

## Usage

```cpp
#include "sensor_reading.h"

SensorReading reading;
reading.temperature = 22.5f;
reading.humidity    = 58.0f;
reading.timestamp   = 1745500000;  // Unix timestamp (UTC)

someHandler.handle(reading);
```

Aggregate initialisation is also valid:

```cpp
SensorReading reading = { 22.5f, 58.0f, 1745500000 };
```

---

## Future Considerations

- If the sensor driver gains error states (e.g. a read failure), an `isValid`
  flag or a separate `SensorError` type could be added here.
- Sub-second precision would require changing `timestamp` to `uint64_t` or
  adding a `milliseconds` field. For per-second sampling this is unnecessary.
