#include "logger.h"
#include <stdio.h>  // snprintf — available on both Arduino and native toolchains
#include <string.h> // strlen

// Global logger instance.
Logger Log;

void Logger::setSink(LogSinkFn fn) { _sink = fn; }
void Logger::setLevel(LogLevel lv) { _minLevel = lv; }
LogLevel Logger::getLevel() const { return _minLevel; }

void Logger::emit(LogLevel level, const char *prefix, const char *msg)
{
  if (_sink == nullptr)
    return;
  if (level < _minLevel)
    return;
  if (msg == nullptr)
    return;

  // Build "[PREFIX] message" in a fixed stack buffer — no heap allocation.
  char buf[LOG_LINE_MAX];
  snprintf(buf, sizeof(buf), "%s %s", prefix, msg);
  _sink(buf);
}

void Logger::debug(const char *msg) { emit(LogLevel::DEBUG, "[DEBUG]", msg); }
void Logger::info(const char *msg) { emit(LogLevel::INFO, "[INFO]", msg); }
void Logger::warn(const char *msg) { emit(LogLevel::WARN, "[WARN]", msg); }
void Logger::error(const char *msg) { emit(LogLevel::ERROR, "[ERROR]", msg); }
