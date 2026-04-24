#pragma once
#include <stdint.h>

/**
 * Represents the active data management mode.
 *
 * Modes control how sensor readings are stored and communicated:
 *   NC   - No Collection: run but discard all readings.
 *   LS   - Local Storage: persist readings to SD card.
 *   SS   - Storage Sync: LS + periodic upload to remote endpoint.
 *   RT   - Real-Time: publish live readings via MQTT.
 *   LSRT - Local Storage + Real-Time: LS and RT simultaneously.
 *   SSRT - Storage Sync + Real-Time: SS and RT simultaneously.
 */
enum class DataMode : uint8_t
{
  NC = 0,
  LS = 1,
  SS = 2,
  RT = 3,
  LSRT = 4,
  SSRT = 5,
  UNKNOWN = 255 // Failed to parse; system should treat as NC
};

/**
 * Parse a DATA_MODE string from .env configuration.
 * Matching is exact and case-sensitive (uppercase only).
 * Returns DataMode::UNKNOWN for null, empty, or unrecognised input.
 */
DataMode parseDataMode(const char *str);

/**
 * Returns true when the mode persists readings to SD card.
 * Applies to: LS, SS, LSRT, SSRT.
 */
bool supportsLocalStorage(DataMode mode);

/**
 * Returns true when the mode publishes live readings via MQTT.
 * Applies to: RT, LSRT, SSRT.
 */
bool supportsRealtime(DataMode mode);

/**
 * Returns true when the mode synchronises stored data to a remote endpoint.
 * Applies to: SS, SSRT.
 */
bool supportsSync(DataMode mode);

/**
 * Returns the mode to activate when network connectivity is lost.
 *
 * Modes that require a network connection fall back to LS so that data
 * is not silently dropped. Modes that are already network-independent
 * remain unchanged.
 *
 *   NC   -> NC    (nothing to change)
 *   LS   -> LS    (already local-only)
 *   SS   -> LS    (stop syncing, keep collecting)
 *   RT   -> LS    (buffer locally to avoid data loss)
 *   LSRT -> LS    (drop MQTT stream, keep local part)
 *   SSRT -> LS    (drop MQTT stream and sync, keep local part)
 *   UNKNOWN -> NC (safe default)
 */
DataMode fallbackModeWhenOffline(DataMode mode);

/** Returns the canonical string representation of a mode (e.g. "LS"). */
const char *dataModeToString(DataMode mode);
