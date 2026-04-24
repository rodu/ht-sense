# ADR 001 — Testing Strategy: Native Host Tests + On-Device Tests, No Simulator

**Date:** 2026-04-24
**Status:** Accepted

---

## Context

The HT-Sense board firmware is written for an Arduino Uno R4 WiFi (Renesas RA4M1).
We needed to decide how to test it:

- **Option A:** Simulate the Arduino environment on a host machine (e.g. using
  a simulator or mocking the Arduino SDK).
- **Option B:** Test only on the real board via serial.
- **Option C:** Split: test pure C++ logic on the host; test hardware-dependent
  code on the real board.

PlatformIO supports all three patterns via its `native` and `uno_r4_wifi`
environments.

---

## Decision

Use **Option C**: a two-layer approach.

1. **Native host tests** (`pio test -e native`) for all modules in `lib/` that
   contain pure C++ logic — no Arduino APIs, no hardware.
2. **On-device tests** (`pio test -e uno_r4_wifi`) for hardware-facing code:
   SD card, WiFi, MQTT, RTC, and sensor reads. Start with a minimal smoke test
   (`test_embedded_smoke`) and grow from there.

No simulator is used.

---

## Rationale

**Why not simulate?**

Simulators for the Uno R4 WiFi are incomplete or non-existent for the
peripherals this project relies on (WiFi stack, SD card, Renesas RA4M1 RTC,
MQTT over TCP). A simulator would require significant maintenance effort to keep
up with library changes and would not reproduce timing-sensitive or
hardware-specific behaviour. The risk of false-green tests is high.

**Why native tests at all?**

The majority of the interesting logic — config parsing, filename generation,
sync checkpoint management, data mode transitions, offline fallback — has no
hardware dependency. Testing this logic on the host is:
- Fast (sub-second feedback).
- Runnable without a board.
- Easy to run in CI.
- Simpler to debug (plain stdout, no serial monitor).

**Why not test only on the board?**

On-device testing requires a physical board, a USB cable, serial upload time
(10–30 seconds per run), and manual observation. For pure logic, this overhead
is unnecessary and slows development significantly.

**The structural consequence**

Separating logic from hardware access is not just good testing practice — it
forces a cleaner architecture. Modules in `lib/` that contain logic must not
call Arduino APIs directly. This makes the codebase more modular and makes
future port or refactor work easier.

---

## Consequences

- All modules in `lib/` must be free of Arduino-specific headers as a condition
  of being testable on `native`.
- Hardware access (SD, WiFi, MQTT, Serial) is mediated through thin adapter
  functions or injected function pointers, not called directly from logic code.
- The `test_embedded_smoke` test exists as the minimal proof that the toolchain,
  upload, and serial test reporting work on the real board.
- As hardware-facing modules are built, they get their own `test_embedded_*`
  folders in the `uno_r4_wifi` environment.
