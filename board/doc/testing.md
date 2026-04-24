# Testing

## Strategy

The project uses two distinct testing layers:

| Layer | Environment | Toolchain | When to use |
|---|---|---|---|
| **Host tests** | `native` | PlatformIO + Unity on macOS/Linux | Pure C++ logic: parsing, state machines, file naming, checkpoints |
| **On-device tests** | `uno_r4_wifi` | PlatformIO + Unity via serial | Hardware-facing code: SD, WiFi, MQTT, RTC, sensor reads |

No simulator is used. The practical split for an Arduino Uno R4 WiFi project is
host tests for logic and the real board for hardware. See
[decisions/001-testing-strategy.md](decisions/001-testing-strategy.md) for the
full rationale.

---

## Running Tests

```bash
# All native (logic) tests — runs on your computer, no board needed
~/.platformio/penv/bin/pio test -e native

# On-board smoke test — board must be connected via USB
~/.platformio/penv/bin/pio test -e uno_r4_wifi
```

If `pio` is on your `PATH`:

```bash
pio test -e native
pio test -e uno_r4_wifi
```

---

## Test Layout

Each module in `lib/` has a corresponding test folder in `test/`:

```
test/
  test_logger/          # native — Logger sink, level filtering, format
  test_data_mode/       # native — parseDataMode, capability flags, fallback
  test_nc_handler/      # native — discard counting, log emission
  test_file_naming/     # native — 8.3 filename and directory generation
  test_sync_checkpoint/ # native — serialise/deserialise checkpoint
  test_embedded_smoke/  # uno_r4_wifi — minimal on-board sanity check
```

Every test folder contains exactly one file: `test_main.cpp`. This is
PlatformIO's convention — each folder is an independent test binary.

---

## Test File Structure

Every `test_main.cpp` follows this template:

```cpp
#include <unity.h>
#include "my_module.h"

void setUp()    { /* reset shared state before each test */ }
void tearDown() { /* cleanup after each test */ }

void test_something() {
    TEST_ASSERT_EQUAL(expected, actual);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_something);
    return UNITY_END();
}
```

For **native** tests, `main()` is a plain C++ entry point.

For **on-device** tests (Arduino), use `setup()` / `loop()` instead:

```cpp
void setup() {
    delay(2000);      // let serial enumerate
    UNITY_BEGIN();
    RUN_TEST(test_something);
    UNITY_END();
}

void loop() {}        // nothing; tests run once in setup()
```

---

## Writing Testable Code

The key rule: **a module is testable on the native environment if and only if
it does not call Arduino APIs directly.**

Guidelines:
- Keep logic in small, self-contained functions that take plain inputs and return
  plain outputs.
- Inject hardware dependencies via function pointers or thin adapters rather
  than calling `Serial`, `SD`, `WiFi`, etc. directly inside logic code.
- The `Logger` module is a good example: `setSink()` accepts a plain function
  pointer, so tests inject a capture function instead of a `Serial` object.
- Mode handlers (`nc_handler`, future `ls_handler`, etc.) should contain only
  mode logic. SD or MQTT calls live in separate adapter classes.

**Rule of thumb:** if you cannot write a test for a function without a physical
board, the function is probably doing too much.

---

## Logger Capture Pattern

Many tests need to verify log output. The standard pattern is to inject a
capture sink in `setUp()` and restore it in `tearDown()`:

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

Then assert on the captured content:

```cpp
void test_nc_emits_debug_log() {
    NcHandler h;
    SensorReading r = { 22.5f, 55.0f, 1000 };
    h.handle(r);
    TEST_ASSERT_NOT_NULL(strstr(capturedLine, "[DEBUG]"));
    TEST_ASSERT_NOT_NULL(strstr(capturedLine, "NC"));
}
```

---

## Adding a New Module

1. Create `lib/<module_name>/<module_name>.h` and `.cpp`.
2. Create `test/test_<module_name>/test_main.cpp`.
3. Decide which environment it belongs to:
   - Pure C++ logic → add it to the `native` env (default; no change needed in
     `platformio.ini` since `test_ignore = test_embedded_smoke` covers everything).
   - Requires hardware → add to `uno_r4_wifi` and update `test_filter` if needed.
4. Add a module reference document to `doc/modules/`.
5. If a non-obvious design decision was made, add an ADR to `doc/decisions/`.

---

## Test Coverage Goals

Each module should have tests covering:

| Area | What to test |
|---|---|
| Happy path | Correct output for every valid input combination |
| Boundary values | Empty strings, zero, `UINT32_MAX`, extreme floats |
| Invalid input | Null pointers, malformed strings, out-of-range values |
| Side effects | Counter increments, log emission, state changes |
| Interaction with Logger | Debug output present/absent based on level |

Tests do **not** need to cover:
- Internal implementation details (test the public API only)
- Hardware-specific timing behaviour (belongs in on-device tests)
- Full integration flows across multiple modules (belongs in smoke tests)
