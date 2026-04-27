#pragma once
#include "data_mode.h"

/**
 * Loads application configuration from the SD card and applies it.
 *
 * Reads /config/config.json from the SD card at the given chip-select pin,
 * parses the JSON using ConfigParser, sets the global logger level, and
 * returns the resolved DataMode.  Falls back to safe defaults (NC mode,
 * INFO log level) when the SD card is absent or the file is missing or
 * corrupt.
 *
 * @param sdCsPin  SPI chip-select pin for the SD card.
 * @return         The DataMode resolved from config, or DataMode::NC on error.
 */
DataMode loadConfig(int sdCsPin);
