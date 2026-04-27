#pragma once
#include <stddef.h> // size_t
#include <stdint.h>

/**
 * Log severity levels, ordered from most to least verbose.
 * NONE suppresses all output.
 */
enum class LogLevel : uint8_t
{
  DEBUG = 0,
  INFO = 1,
  WARN = 2,
  ERROR = 3,
  NONE = 4
};

/**
 * Sink callback type.
 * Receives a fully-formatted log line (e.g. "[INFO] System started").
 * The line does not include a trailing newline — the sink is responsible
 * for any required line ending.
 */
using LogSinkFn = void (*)(const char *line);

/**
 * Lightweight, allocation-free logger.
 *
 * Usage:
 *   // In setup() on Arduino:
 *   Log.setSink([](const char *line) { Serial.println(line); });
 *   Log.setLevel(LogLevel::INFO);
 *
 *   // In any module:
 *   Log.info("Sensor initialised");
 *   Log.warn("SD card not found");
 *
 * The sink is a plain function pointer so the logger introduces zero
 * heap allocation and no Arduino-specific dependencies.
 *
 * When no sink is set, all log calls are no-ops (safe to call before setup).
 * Messages below the current minimum level are filtered before the sink is
 * called, so filtering is always O(1).
 *
 * The internal format buffer is LOG_LINE_MAX bytes; messages that exceed
 * that length are silently truncated.
 */
class Logger
{
public:
  static constexpr size_t LOG_LINE_MAX = 128;

  /** Replace the active output sink. Pass nullptr to silence all output. */
  void setSink(LogSinkFn fn);

  /** Set the minimum severity level. Messages below this level are dropped. */
  void setLevel(LogLevel minLevel);

  /** Return the current minimum severity level. */
  LogLevel getLevel() const;

  void debug(const char *msg);
  void info(const char *msg);
  void warn(const char *msg);
  void error(const char *msg);

private:
  LogLevel _minLevel = LogLevel::INFO;
  LogSinkFn _sink = nullptr;

  void emit(LogLevel level, const char *prefix, const char *msg);
};

/**
 * Global logger instance.
 * Configure once in setup(); use from any translation unit via #include "logger.h".
 */
extern Logger Log;

/**
 * Parse a LOG_LEVEL string from .env configuration into a LogLevel value.
 * Accepted strings (exact, uppercase): DEBUG, INFO, WARN, ERROR, NONE.
 * Returns LogLevel::INFO for null, empty, or unrecognised input.
 */
LogLevel logLevelFromString(const char *str);
