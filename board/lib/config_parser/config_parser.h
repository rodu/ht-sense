#pragma once
#include <stddef.h>
#include <stdint.h>

/**
 * Flat struct holding all application configuration parameters.
 *
 * All fields are null-terminated C strings. Always initialise from
 * defaultConfig() before attempting to load from storage so that safe
 * values are in place even when the SD card is absent or the file is corrupt.
 */
struct AppConfig
{
  char dataMode[8];      ///< "NC" | "LS" | "SS" | "RT" | "LSRT" | "SSRT"
  char logLevel[8];      ///< "DEBUG" | "INFO" | "WARN" | "ERROR" | "NONE"
  char wifiSsid[64];     ///< wifi.ssid
  char wifiPassword[64]; ///< wifi.password  (never log this field)
  char mqttBroker[64];   ///< mqtt.broker (hostname or IP)
  char mqttTopic[64];    ///< mqtt.topic
  char syncUrl[128];     ///< sync.url
  char syncToken[64];    ///< sync.token (never log this field)
};

/**
 * Returns an AppConfig pre-populated with safe, hardcoded defaults.
 * Call this before loadConfig() so the board continues to operate if
 * the SD card is absent or the JSON file is corrupt.
 */
AppConfig defaultConfig();

/**
 * JSON-based configuration parser.
 *
 * Uses the "Default & Override" pattern:
 *   1. Populate cfg with defaultConfig().
 *   2. Call parse() to overlay only the fields present in the JSON.
 *
 * Config file format — /config/config.json on the SD card:
 * @code
 * {
 *   "dataMode": "NC",
 *   "logLevel": "INFO",
 *   "wifi":  { "ssid": "...", "password": "..." },
 *   "mqtt":  { "broker": "192.168.1.1", "topic": "ht-sense/data" },
 *   "sync":  { "url": "https://...", "token": "..." }
 * }
 * @endcode
 *
 * Integrity is verified externally: compute crc32() over the raw JSON text
 * and compare it with the hex value stored in /config/config.crc before
 * calling parse(). If the CRC does not match, discard the buffer and keep
 * the defaults.
 */
class ConfigParser
{
public:
  /**
   * Parse a JSON config buffer, overriding fields present in the document.
   *
   * @param jsonText  Pointer to the JSON text (need not be null-terminated
   *                  beyond @p len bytes).
   * @param len       Number of bytes to parse.
   * @param cfg       Config struct to update in place.
   * @returns         true  – JSON was valid and at least one field applied.
   *                  false – input was null/empty or JSON could not be parsed.
   */
  bool parse(const char *jsonText, size_t len, AppConfig &cfg) const;

  /**
   * Compute a CRC32 (ISO 3309 / PKZIP) checksum over a data buffer.
   *
   * Use this to verify a config file's integrity before parsing:
   *   1. Read the raw JSON bytes.
   *   2. Compute crc32() over those bytes.
   *   3. Compare with the 8-digit hex value stored in config.crc.
   *   4. Call parse() only if they match.
   *
   * @param data  Pointer to the data buffer (may be nullptr when len == 0).
   * @param len   Number of bytes to process.
   * @returns     CRC32 value.
   */
  static uint32_t crc32(const uint8_t *data, size_t len);
};
