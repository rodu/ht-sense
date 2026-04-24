#pragma once
#include <stdint.h>
#include "sensor_reading.h"

/**
 * Handler for the NC (No Collection) data mode.
 *
 * In NC mode the firmware runs normally — sensors are read, the loop
 * executes — but every reading is immediately discarded. No data is
 * written to SD, streamed over MQTT, or queued for sync.
 *
 * This is useful during development (inspect values over serial) and
 * for isolating non-data-handling faults without the overhead of
 * storage or networking code.
 *
 * The handler keeps a running count of discarded readings so that
 * callers can verify activity and produce diagnostic output.
 */
class NcHandler
{
public:
  /**
   * Discard a reading.
   * The reading is intentionally not forwarded anywhere. A DEBUG-level
   * log line is emitted via the global Log instance so the discard is
   * visible when debug logging is active.
   * Increments the internal discard counter.
   */
  void handle(const SensorReading &reading);

  /** Total readings discarded since construction or the last resetCount(). */
  uint32_t discardedCount() const;

  /** Reset the discard counter to zero. */
  void resetCount();

private:
  uint32_t _discardedCount = 0;
};
