# Module: nc_handler

**Location:** `lib/nc_handler/`
**Files:** `nc_handler.h`, `nc_handler.cpp`
**Environment:** native + uno_r4_wifi (no Arduino dependency)
**Tests:** `test/test_nc_handler/test_main.cpp`

---

## Purpose

Implements the **NC (No Collection)** data mode handler. In NC mode the
firmware reads the sensor and runs the normal loop, but every reading is
immediately discarded. Nothing is written to SD, sent over MQTT, or queued
for sync.

NC mode is useful for:
- Development — observe sensor values over Serial without any storage overhead.
- Fault isolation — disable data handling entirely to rule it out when debugging
  unrelated issues (WiFi, RTC, SD card).

---

## API

```cpp
class NcHandler {
public:
    // Discard a reading. Increments counter. Emits Log.debug().
    void handle(const SensorReading &reading);

    // Total readings discarded since construction or last resetCount().
    uint32_t discardedCount() const;

    // Reset the counter to zero.
    void resetCount();
};
```

---

## Behaviour

`handle()` does exactly three things, in order:

1. **Discards the reading.** The `reading` parameter is cast to `(void)` to
   suppress any unused-parameter warning. No field of the reading is accessed
   or stored.
2. **Increments `_discardedCount`.**
3. **Calls `Log.debug("NC: reading discarded")`.**  The debug line is only
   emitted if the global `Log` sink is set and the current log level is
   `DEBUG` or lower. If either condition is not met, the call is a no-op
   (handled inside `Logger`).

No other side effects exist.

---

## Usage

```cpp
#include "nc_handler.h"
#include "sensor_reading.h"

NcHandler ncHandler;

// In the main loop:
SensorReading reading = sensor.read();
ncHandler.handle(reading);

// Diagnostic output (optional):
Log.info("Discarded readings: ");   // combine with itoa / snprintf as needed
```

---

## Logging Integration

`NcHandler` depends on the global `Log` instance from `lib/logger/`. It only
emits at `DEBUG` level, so in normal production operation (where the log level
is `INFO`) there is no serial output per reading — only the counter increments.

To see per-reading debug lines:

```cpp
Log.setLevel(LogLevel::DEBUG);
```

---

## Test Coverage

12 Unity tests across 2 groups:

| Group | Cases |
|---|---|
| Discard counting | Initial zero, single increment, multiple increments, reset, post-reset count, repeated resets, counter is only observable state |
| Logging behaviour | Emits DEBUG log, log contains "NC", one log per reading, suppressed when level > DEBUG, no crash when sink is null |

The test that verifies "counter is only observable state" feeds readings with
extreme values (`0.0f` and `100.0f`, timestamp `0` and `UINT32_MAX`) and
confirms that only `discardedCount()` changes — establishing that the handler
genuinely discards rather than storing anything.

---

## Design Notes

- `NcHandler` holds **no reference** to the sensor reading. It receives it by
  `const &` purely to match the handler interface that all future mode handlers
  will share.
- The discard counter exists only to give callers (tests, diagnostics) a way to
  verify the handler is being called. It has no effect on functionality.
- `resetCount()` is provided primarily for test hygiene (resetting between
  test cases) but may also be useful for runtime statistics resets.
