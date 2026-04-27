# Architecture

## Overview

HT-Sense is an Arduino Uno R4 WiFi firmware that reads humidity and temperature
from a sensor and manages that data according to a configurable _data mode_.
The mode is set in a `.env` file on the SD card and controls whether readings
are discarded, stored locally, synced to a remote endpoint, streamed live over
MQTT, or a combination of those.

The firmware is structured as a collection of small, independently testable
C++ modules under `lib/`. `src/main.cpp` is intentionally thin — it wires
modules together and owns the Arduino `setup()` / `loop()` contract, but
contains no business logic of its own.

---

## Project Layout

```
board/
  platformio.ini          # Build & test environments (uno_r4_wifi, native)
  src/
    main.cpp              # Thin entry point — wires modules, owns setup/loop
  include/
    sensor_reading.h      # Cross-cutting: shared SensorReading struct
  lib/
    logger/               # Cross-cutting: allocation-free serial logger
    data_mode/            # Parse DATA_MODE; compute capability flags
    nc_handler/           # NC mode — discard readings, log, count
    file_naming/          # Generate 8.3-safe SD paths from timestamps
    sync_checkpoint/      # Track last-synced file and byte offset
    config_parser/        # Parse .env key=value text into a config struct
  test/
    test_logger/          # native env
    test_data_mode/       # native env
    test_nc_handler/      # native env
    test_file_naming/     # native env
    test_sync_checkpoint/ # native env
    test_embedded_smoke/  # uno_r4_wifi env — on-board sanity check
  doc/                    # This documentation folder
```

---

## Module Dependency Map

Modules are layered. Lower rows depend only on rows above them.

```
┌──────────────────────────────────────────────────────────────────┐
│  Cross-cutting (no dependencies on other project modules)        │
│  logger            sensor_reading                                │
└───────────────────────────┬──────────────────────────────────────┘
                            │
┌───────────────────────────▼──────────────────────────────────────┐
│  Domain logic (depends on logger and/or sensor_reading)          │
│  data_mode         nc_handler         config_parser              │
│  file_naming       sync_checkpoint                               │
└───────────────────────────┬──────────────────────────────────────┘
                            │
┌───────────────────────────▼──────────────────────────────────────┐
│  Entry point                                                     │
│  src/main.cpp  (wires everything together)                       │
└──────────────────────────────────────────────────────────────────┘
```

Rules:
- `logger` and `sensor_reading` have **no** dependencies on other project modules.
  They may depend only on the C/C++ standard library and Arduino built-ins.
- Domain modules may depend on `logger` and `sensor_reading` but **not** on
  each other (avoids coupling and keeps tests simple).
- `main.cpp` is the only place that wires multiple modules together.

---

## Data Flow

```
  [Sensor hardware]
        │
        ▼
  loop() reads SensorReading{temperature, humidity, timestamp}
        │
        ▼
  Dispatch to active mode handler
  ┌─────┴─────────────────────────────────┐
  │ NC      │ LS     │ RT     │ SS / LSRT │ ...
  │ discard │ SD     │ MQTT   │ SD + sync │
  └─────────┴────────┴────────┴───────────┘
        │
        ▼
  Log.debug / Log.info / Log.warn / Log.error  →  Serial (or other sink)
```

---

## main.cpp Responsibilities

`src/main.cpp` is the only translation unit that includes Arduino headers
(other than mode handlers that will later need them for SD/WiFi/MQTT).

Current wiring (as of initial implementation):

```cpp
// setup()
Serial.begin(115200);
Log.setSink([](const char *line) { Serial.println(line); });
Log.setLevel(LogLevel::INFO); // default; overridden by applyConfig()
applyConfig();                // reads SD .env → applies LOG_LEVEL + DATA_MODE

// loop()
SensorReading reading = { /* from sensor */ };
switch (activeMode) {
    case DataMode::NC: ncHandler.handle(reading); break;
    ...
}
```

`applyConfig()` opens `/.env` (falling back to `/config/.env`) from the SD card,
passes the text to `ConfigParser::findValue`, and applies `LOG_LEVEL` via
`logLevelFromString()` and `DATA_MODE` via `parseDataMode()`. When no SD card is
present the firmware logs a warning and continues with safe defaults (`INFO` / `NC`).

---

## Configuration: .env File

All behaviour and credential parameters are stored in a `.env` file on the SD
card (`/config/.env` or root `.env`). Format:

```
DATA_MODE=NC
LOG_LEVEL=INFO
WIFI_SSID=your_network
WIFI_PASSWORD=secret
MQTT_BROKER=192.168.1.100
MQTT_TOPIC=ht-sense/data
SYNC_URL=https://example.com/api/upload
SYNC_TOKEN=your_token
```

Lines starting with `#` are comments. The `config_parser` module is responsible
for reading and validating this file. Credentials are never echoed to logs.

`LOG_LEVEL` accepts `DEBUG`, `INFO`, `WARN`, `ERROR`, or `NONE` (default: `INFO`).
To change the log level, edit this key on the SD card — no firmware recompile needed.

---

## Data Modes

| Mode | Storage | MQTT stream | Remote sync | Offline behaviour |
|------|---------|-------------|-------------|-------------------|
| NC   | No      | No          | No          | N/A               |
| LS   | SD card | No          | No          | N/A (local only)  |
| SS   | SD card | No          | Yes         | Falls back to LS  |
| RT   | No      | Yes         | No          | Falls back to LS  |
| LSRT | SD card | Yes         | No          | Falls back to LS  |
| SSRT | SD card | Yes         | Yes         | Falls back to LS  |

When network connectivity is lost, any mode that requires it degrades gracefully
to `LS` so readings are not dropped. See
[decisions/003-data-mode-fallback.md](decisions/003-data-mode-fallback.md).

---

## SD Card Filename Policy

Files on the SD card use strict **8.3 format** for maximum FAT16/FAT32
compatibility. Directory structure is date-based:

```
YYYY/MM/DD/YYMMDDHH.CSV
```

Example: `2026/04/24/26042415.CSV`

Collision handling appends a short sequence suffix while staying within 8.3:
`240424A1.CSV`, `240424A2.CSV`, …

See the `file_naming` module for implementation details.

---

## PlatformIO Environments

| Environment | Platform | Purpose | Run command |
|---|---|---|---|
| `native` | Host (macOS/Linux) | Fast unit tests for pure C++ logic | `pio test -e native` |
| `uno_r4_wifi` | Renesas RA / Arduino | Build firmware + on-board smoke test | `pio test -e uno_r4_wifi` |

The `native` environment excludes `test_embedded_smoke`.
The `uno_r4_wifi` environment runs **only** `test_embedded_smoke`.
