# ADR 002 — Logger Design: Function-Pointer Sink, No Heap, No Arduino Dependency

**Date:** 2026-04-24
**Status:** Accepted

---

## Context

The firmware needs a shared logger that:

- Can be used from any module (`data_mode`, `nc_handler`, future SD/WiFi/MQTT
  modules) without them needing to know about `Serial` or any other output target.
- Is usable in native host tests without mocking the Arduino SDK.
- Introduces no heap allocation (embedded RAM is limited; `String` and `std::string`
  are avoided throughout the firmware).
- Is easy to silence or redirect (e.g. to a different output target in future).

Several approaches were considered:

| Approach | Heap? | Arduino dep? | Testable native? | Redirectable? |
|---|---|---|---|---|
| `Serial.print` directly in modules | No | Yes | No | No |
| `Serial` passed as reference/pointer | No | Yes | No (needs mock) | Yes |
| `std::function` sink | Yes | No | Yes | Yes |
| Plain function-pointer sink | No | No | Yes | Yes |
| Stream-based (Arduino `Stream`) | No | Yes | No | Partial |

---

## Decision

Use a **plain function-pointer sink** (`void (*)(const char*)`).

- The `Logger` class holds a `LogSinkFn` (a `using` alias for the function
  pointer type).
- In `setup()` the caller injects the real sink: `Log.setSink([](const char *l)
  { Serial.println(l); });`
- In tests the caller injects a capture function that writes to a stack buffer.
- Passing `nullptr` silences all output.

Message formatting uses `snprintf` into a fixed 128-byte stack buffer
(`LOG_LINE_MAX`). No `String`, no `std::string`, no heap.

---

## Rationale

**Why not `std::function`?**

`std::function` supports capturing lambdas, which is more ergonomic, but it
performs heap allocation for non-trivial captures. That is unacceptable on an
Arduino with limited RAM. A plain function pointer covers all needed cases if
any state that would be captured is stored in a module-level static variable
instead.

**Why not pass `Serial` as a dependency?**

Passing `Stream&` or a `Print&` reference would require including Arduino
headers in the logger, making it impossible to compile for the `native`
environment without a mock. The function pointer approach inverts the dependency:
the logger knows nothing about `Serial`; the caller decides what to do with the
line.

**Why a global `Log` instance rather than passing a logger reference?**

Passing a logger reference to every module function would add boilerplate to
every call site. A global instance is idiomatic for loggers in embedded C++
and matches how popular Arduino logging libraries (ArduinoLog, ESP_LOG) work.
The single global instance is defined in `logger.cpp`; all other translation
units see it via `extern Logger Log` in `logger.h`.

**Why 128 bytes for `LOG_LINE_MAX`?**

128 bytes fits the longest expected log line on the Serial monitor while staying
well within stack budget on the RA4M1 (8 KB RAM stack area). It can be increased
if needed by changing the constant in `logger.h`.

---

## Consequences

- Capturing lambdas cannot be used as sinks. If a sink needs associated state,
  a module-level static variable must be used.
- Log lines longer than 128 bytes (including the prefix) are silently truncated.
  This is considered acceptable — very long log lines indicate a design smell
  in the calling code.
- The global `Log` instance means tests that use the logger must restore it to
  a known state in `tearDown()` to avoid cross-test interference.
