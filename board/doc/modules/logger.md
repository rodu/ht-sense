# Module: logger

**Location:** `lib/logger/`
**Files:** `logger.h`, `logger.cpp`
**Environment:** native + uno_r4_wifi (no Arduino dependency)
**Tests:** `test/test_logger/test_main.cpp`

---

## Purpose

A lightweight, allocation-free logger that can be used from any module in the
firmware. It introduces zero heap allocation and has no dependency on any Arduino
library, making it fully testable on the native (host) environment.

Output is routed through an injectable _sink_ — a plain function pointer — so
the destination (Serial, a buffer, a no-op) is decided at runtime by the caller.

---

## Design Highlights

- **No heap allocation.** The formatted line is assembled in a fixed 128-byte
  stack buffer (`LOG_LINE_MAX`). Messages longer than this are silently truncated.
- **No Arduino dependency.** The header includes only `<stddef.h>` and
  `<stdint.h>`. This keeps it usable in native unit tests without any mocking.
- **Injectable sink.** The sink is a plain `void (*)(const char*)` function
  pointer. Tests inject a capture function; production code injects a lambda
  that calls `Serial.println`.
- **Global instance.** A single `extern Logger Log` is declared in the header
  and defined in `logger.cpp`. All modules include `logger.h` and call
  `Log.info(...)` etc. directly.

---

## API

```cpp
enum class LogLevel : uint8_t {
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3,
    NONE  = 4   // suppresses all output
};

using LogSinkFn = void (*)(const char *line);

class Logger {
public:
    static constexpr size_t LOG_LINE_MAX = 128;

    void setSink(LogSinkFn fn);        // nullptr silences all output
    void setLevel(LogLevel minLevel);  // default: INFO
    LogLevel getLevel() const;

    void debug(const char *msg);
    void info (const char *msg);
    void warn (const char *msg);
    void error(const char *msg);
};

extern Logger Log;  // global instance
```

---

## Output Format

Each line follows the pattern:

```
[LEVEL] message body
```

Examples:

```
[DEBUG] NC: reading discarded
[INFO]  HT-Sense starting
[WARN]  SD card not found
[ERROR] WiFi connection failed
```

The sink receives the formatted string **without** a trailing newline.
`Serial.println` adds the newline on Arduino; adjust the sink if a different
output target is used.

---

## Usage

### In `setup()` (Arduino)

```cpp
#include "logger.h"

void setup() {
    Serial.begin(115200);
    while (!Serial) {}

    Log.setSink([](const char *line) { Serial.println(line); });
    Log.setLevel(LogLevel::INFO);   // suppress DEBUG in production
    Log.info("System starting");
}
```

### In any module

```cpp
#include "logger.h"

void myFunction() {
    Log.debug("entered myFunction");
    // ...
    Log.warn("something unusual");
}
```

### In tests (capture pattern)

```cpp
static char capturedLine[Logger::LOG_LINE_MAX];
static int  captureCount = 0;

static void captureOutput(const char *line) {
    strncpy(capturedLine, line, sizeof(capturedLine) - 1);
    capturedLine[sizeof(capturedLine) - 1] = '\0';
    captureCount++;
}

void setUp() {
    capturedLine[0] = '\0';
    captureCount    = 0;
    Log.setSink(captureOutput);
    Log.setLevel(LogLevel::DEBUG);
}

void tearDown() {
    Log.setSink(nullptr);
    Log.setLevel(LogLevel::INFO);
}
```

---

## Test Coverage

The test file covers:

| Group | Cases |
|---|---|
| Sink management | null sink no-crash, sink called on pass, runtime sink swap |
| Level filtering | at threshold, above, below, NONE suppresses all, DEBUG passes all |
| Output format | correct prefix per level, message body present, both together |
| Edge cases | null message, empty message, long message truncated safely, once-per-call |
| Level changes | take effect immediately |

---

## Constraints and Limitations

- `LOG_LINE_MAX` is 128 bytes. Combined prefix + space + message that exceeds
  this is silently truncated. This is intentional to avoid any dynamic allocation.
- The sink is a plain function pointer. Lambdas that capture state cannot be
  used as sinks; use a module-level static variable if state capture is needed.
- Thread safety is not relevant on a single-core Arduino, but the logger is not
  thread-safe if ever ported to a multi-core target.
