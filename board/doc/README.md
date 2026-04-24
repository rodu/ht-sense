# HT-Sense Board — Documentation

This folder captures the design, architecture, and decisions made during development
of the Arduino firmware for the HT-Sense project.

It is written to be useful in two ways:

- **Human reading** — browse it any time you need a refresher on why something was
  built the way it was, or to onboard a collaborator.
- **AI context** — the documents follow a consistent structure so an AI assistant
  can read them as background before suggesting changes to the codebase.

---

## Contents

### Architecture

| Document | What it covers |
|---|---|
| [architecture.md](architecture.md) | Overall system structure, module map, data flow, and `main.cpp` wiring |
| [testing.md](testing.md) | Test strategy, PlatformIO environments, how to add new tests |

### Module References

One document per library module. Each describes the module's purpose, public API,
usage examples, and what tests cover it.

| Document | Module |
|---|---|
| [modules/logger.md](modules/logger.md) | `lib/logger` — allocation-free serial logger |
| [modules/sensor_reading.md](modules/sensor_reading.md) | `lib/sensor_reading` — shared `SensorReading` struct |
| [modules/data_mode.md](modules/data_mode.md) | `lib/data_mode` — data mode parsing, capability flags, offline fallback |
| [modules/nc_handler.md](modules/nc_handler.md) | `lib/nc_handler` — NC mode handler |

### Architecture Decision Records (ADRs)

ADRs record *why* a significant decision was made. They are short, permanent, and
never deleted — if a decision is later reversed, a new ADR records the reversal.

| Document | Decision |
|---|---|
| [decisions/001-testing-strategy.md](decisions/001-testing-strategy.md) | Native host tests + on-device tests; no simulator |
| [decisions/002-logger-design.md](decisions/002-logger-design.md) | Function-pointer sink; no heap; no Arduino dependency |
| [decisions/003-data-mode-fallback.md](decisions/003-data-mode-fallback.md) | Network modes fall back to LS, never NC, to avoid data loss |

---

## Conventions

- Module documents are updated whenever the module's public API changes.
- A new ADR is created for every decision that is non-obvious, has trade-offs,
  or is likely to be questioned in future.
- ADRs are **never edited retroactively** to appear prescient — the date and
  context at the time of writing are part of the record.
