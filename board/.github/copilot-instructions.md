# Arduino HT-Sense Project

This is an arduino project that uses a Humidity and Temperature sensor to collect
environment data.

## Project Structure

```
board/
  platformio.ini
  include/
    sensor_reading.h     # Cross-cutting: shared SensorReading struct (header-only)
  src/
    main.cpp              # Thin entry point — wires modules, no logic
  lib/
    logger/               # Cross-cutting: allocation-free serial logger
      logger.h
      logger.cpp
    data_mode/
      data_mode.h
      data_mode.cpp
    nc_handler/
      nc_handler.h
      nc_handler.cpp
    file_naming/
      file_naming.h
      file_naming.cpp
    sync_checkpoint/
      sync_checkpoint.h
      sync_checkpoint.cpp
    config_parser/
      config_parser.h
      config_parser.cpp
  test/
    test_logger/          # native env — pure logic, no Arduino APIs
      test_main.cpp
    test_data_mode/       # native env — pure logic, no Arduino APIs
      test_main.cpp
    test_nc_handler/      # native env — pure logic, no Arduino APIs
      test_main.cpp
    test_file_naming/     # native env — pure logic, no Arduino APIs
      test_main.cpp
    test_sync_checkpoint/ # native env — pure logic, no Arduino APIs
      test_main.cpp
    test_embedded_smoke/  # uno_r4_wifi env — minimal on-board sanity check
      test_main.cpp
```

### PlatformIO Environments

Two environments are defined in `platformio.ini`:

- `native` — runs host-side tests for pure C++ logic (no Arduino dependency). Use for all modules in `lib/`. Run with `pio test -e native`.
- `uno_r4_wifi` — runs on-board tests via serial for hardware-facing code. Run with `pio test -e uno_r4_wifi`.

### Module Responsibilities

| Module | Responsibility |
|---|---|
| `logger` | Allocation-free, level-filtered logger with injectable sink. Global `Log` instance used by all modules. Configure once in `setup()` with a Serial sink. |
| `sensor_reading` | Header-only `SensorReading` struct (temperature, humidity, timestamp). Shared across all mode handlers. |
| `data_mode` | Parse `DATA_MODE` config value; compute capability flags and offline fallback |
| `nc_handler` | NC mode handler — discards every reading, emits a DEBUG log, counts discards |
| `file_naming` | Generate 8.3-safe SD card paths and filenames from timestamps |
| `sync_checkpoint` | Serialise/deserialise the last-synced file path and byte offset |
| `config_parser` | Parse `.env` key-value text into a config struct |

## Configuration File Format & Location

- All configuration parameters (behavioral and credentials) are stored in a `.env` file located at the project root or `/config/` directory on the SD card.
- The `.env` file uses standard key-value pairs, one per line:
  ```
  DATA_MODE=LS
  LOG_LEVEL=INFO
  WIFI_SSID=YOUR_SSID
  WIFI_PASSWORD=YOUR_PASSWORD
  MQTT_BROKER=192.168.1.100
  MQTT_TOPIC=ht-sense/data
  SYNC_URL=https://example.com/api/upload
  SYNC_TOKEN=your_api_token
  ```
- `LOG_LEVEL` accepts `DEBUG`, `INFO`, `WARN`, `ERROR`, or `NONE` (default: `INFO`). It is applied at boot by `applyConfig()` in `main.cpp` using `logLevelFromString()` from `logger.h`.
- Lines starting with `#` are treated as comments.
- Credentials (WiFi, API tokens) are stored in plain text by default. For production, consider using EEPROM or encrypted storage for sensitive values.
- On startup, the firmware loads the `.env` file and applies configuration. If the file is missing or malformed, the system falls back to safe defaults and logs an error.

## Unit Testing Framework & Organization

- All modules must include unit tests to verify correctness.
- The project uses the [PlatformIO Unity Test Framework](https://docs.platformio.org/en/latest/frameworks/unity.html) for C++ unit testing.
- Unit tests are placed in the `test/` directory at the project root. Each module should have a corresponding test file (e.g., `test/module_name/test_module_name.cpp`).
- Tests are run using PlatformIO’s `pio test` command.
- All new features and bug fixes must include or update relevant tests.
- Tests should cover:
  - Core logic and edge cases for each module
  - Data mode transitions and fallback logic
  - File naming, directory creation, and collision handling
  - Sync checkpointing and error handling

## Error Handling & Logging

- All modules must implement robust error handling for hardware failures (e.g., SD card, WiFi, sensor read errors) and configuration issues.
- Errors and warnings are logged to the serial console with clear, human-readable messages.
- Critical errors (e.g., SD card not detected, configuration file missing) should trigger a visible indicator (e.g., onboard LED blink pattern) and halt affected operations.
- Non-critical errors (e.g., temporary WiFi loss) should be retried with exponential backoff and logged.
- For persistent errors, the system should attempt to recover gracefully and continue operating in a degraded mode if possible.
- A debug mode can be enabled via configuration to increase log verbosity for troubleshooting.
- An easy-to-use logger should be implemented or integrated to allow logging from any software routine as needed.

## Time Synchronization

- Accurate timestamps are required for file naming, data logging, and synchronization.
- The board uses its onboard real-time clock (RTC) for all timekeeping.
- The RTC should be set manually at deployment or via a configuration routine.
- Optionally, NTP (Network Time Protocol) synchronization can be enabled by setting `ENABLE_NTP=true` in the `.env` file.
  - When enabled and WiFi is available, the system synchronizes time from the NTP server specified in `NTP_SERVER` (default: `pool.ntp.org`).
  - NTP synchronization is implemented using a reliable third-party library.
- All timestamps in logs and filenames must be in UTC.
- If time cannot be determined, the system logs a warning and uses a default timestamp (e.g., 1970-01-01) until time is set.

## Security: Credential Storage & Access

- Credentials such as `WIFI_SSID`, `WIFI_PASSWORD`, and `SYNC_TOKEN` are stored in the `.env` file by default.
- For development and prototyping, plain text storage is acceptable.
- For production or sensitive deployments:
  - Store credentials in EEPROM or encrypted storage if available.
  - Access to credentials should be restricted to initialization routines; do not expose them in logs or over serial output.
  - The `.env` file should not be included in version control or distributed firmware images.
- If EEPROM or secure storage is enabled, set `USE_SECURE_STORAGE=true` in the `.env` file. The firmware will then attempt to load credentials from secure storage first, falling back to `.env` only if unavailable.

## OTA (Over The Air) Updates

- The firmware supports OTA updates to allow remote software upgrades.
- OTA updates can be performed over WiFi or Bluetooth (implementation details TBD).
- OTA functionality is disabled by default and can be enabled by setting `ENABLE_OTA=true` in the `.env` file.
- When enabled, the system periodically checks for updates from a configurable OTA server URL (e.g., `OTA_URL=https://example.com/firmware.bin`).
- The update process must verify the integrity of the downloaded firmware (e.g., checksum or signature verification) before applying.
- On failure, the system must revert to the previous firmware version if possible.
- The OTA update mechanism should be modular to support different transport protocols in the future.

## API/Sync Endpoint Specification

- In `SS` and `SSRT` modes, the board synchronizes data to a remote HTTP endpoint.
- The sync endpoint URL is specified in the `.env` file as `SYNC_URL`.
- Authentication is handled via an API token set in the `.env` file as `SYNC_TOKEN`. The token is sent as an HTTP header (e.g., `Authorization: Bearer <SYNC_TOKEN>`).
- Data is uploaded as CSV files using HTTP POST requests with `Content-Type: text/csv`.
- The API endpoint should return a 2xx status code on success. On failure (4xx/5xx), the board retries the upload according to a backoff policy.
- Only new data (delta since last sync) is uploaded. The board tracks the last synced file and byte/line offset.
- The API should respond with a JSON payload indicating success or failure and may include a new sync checkpoint.
- All network errors and sync failures are logged for debugging.

## Concurrency & Resource Management

- The firmware must avoid blocking operations in the main loop to ensure sensor readings and communication remain responsive.
- Long-running tasks (e.g., file I/O, network sync) should be performed asynchronously or in small steps, using state machines, cooperative multitasking, or a reliable third-party async/task management library (e.g., [TaskScheduler](https://github.com/arkhipenko/TaskScheduler), [Protothreads](https://github.com/danielwagener/arduino-protothreads), or similar).
- Only one file operation (read/write) should be active at a time to prevent SD card corruption.
- RAM and flash usage must be monitored to avoid overflows; buffer sizes should be kept minimal and configurable.
- If a resource constraint is detected (e.g., low memory, SD full), the system logs a warning and attempts to recover gracefully.
- All concurrency and resource management strategies must be covered by unit tests where feasible.

The project also implements management of the collected data and uses configuration
parameters to choose between different data management modes.

## Configuration

The software supports configuration parameters to determine some specific behaviour,
and other more sensitive parameters related to credential storage.

### Behavior

The behavior configuration relates to the software modus operandi. These data
are not intrinsecally sensitive but need to be correct (an acceptable range values)
since they are going to determing the execution mode and final behavior of the
software at run-time.

### Credentials

Credentials to be stored are related to WiFi connectivity SSID and Passowrd,
or Bluetooth parining/autentication secrets (where necessary).

Other type of credentials like API tokens may also need to be managed for
authenticating the board software with any remote API in the future.

## Communication

The Arduino board will communicate with the outside world mostly using wireless
communications such has WiFi and Bluetooth.

WiFi will be used to upload and synchronise data. Bluethooth on the other hand
can be used to synch data in those cases where a reliable WiFi connection is not
available.

given order (1 to 6).

## Data Management

The project collects sensor data and exposes it in one of these modes:

1. NC - No Collection
2. LS - Local Storage
3. SS - Storage Sync
4. RT - Real-Time stream
5. LSRT - Local Storage + Real-Time stream
6. SSRT - Storage Sync + Real-Time stream

The selected mode is controlled by a configuration parameter DATA_MODE with one of: NC, LS, SS, RT, LSRT, SSRT.

### SD Card Storage Rules (applies to LS, SS, LSRT, SSRT)

The default storage target is FAT16/FAT32 on SD, using Arduino-compatible SD access.

1. **Filename compatibility requirement**
  - Default assumption: strict 8.3 filename compatibility for maximum portability.
  - Base name: max 8 characters.
  - Extension: max 3 characters.
  - Preferred extension: CSV.

2. **Character and case convention**
  - Use uppercase names only.
  - Use only A-Z, 0-9, and underscore.
  - Avoid spaces and special symbols.
  - Use one dot only, before the extension.

3. **Timestamp naming policy**
  - Do not use long names like YYYYMMDDHH.csv in strict mode because they exceed 8.3.
  - Use compact names such as:
    - YYMMDDHH.CSV for hourly files
    - YYMMDDMM.CSV for minute-level buckets if needed
  - Keep all naming deterministic and sortable.

4. **Directory strategy**
  - Keep hierarchy shallow and date-based:
    - YYYY/MM/DD/
  - Example:
    - 2026/04/24/24042415.CSV
  - This preserves readability while keeping file names 8.3-safe.

5. **Collision handling**
  - If a generated filename already exists for the same bucket, append or encode a short sequence while still respecting 8.3.
  - Example pattern:
    - 240424A1.CSV
    - 240424A2.CSV

6. **Long filename support (optional)**
  - Long filenames are allowed only if explicitly using a library/configuration that guarantees LFN support on target hardware.
  - If LFN is enabled, still keep names short where possible for RAM and compatibility reasons.
  - 8.3 remains the default policy unless project configuration explicitly opts into LFN.


### NC - No Collection

When using the `NC` data mode the software will still run but no data will be
communicated or stored.

This modality will be useful during development where data can be inspected via the
serial monitor, or for debugging purposes.

This modality allows to exclude pieces of functionality of the software at run-time
in order to isolate and help diagnose any other anomaly, not related to data handling.

### LS - Local Storage

When using LS mode, sensor data is persisted on SD card in CSV format.

For handling the file storage in sub-folders, the [DataLogger](https://github.com/PowerBroker2/DataLogger) library will be used.

File breakdown should be selected based on sampling frequency and payload size. The default mapping is:

| Frequency | File Breakdown |
|---|---|
| Per millisecond | 10 seconds |
| Per second | Hourly |
| Per minute | Daily |
| Per hour | Monthly |
| Per day | Yearly |

Note:
- The mapping may be adjusted to keep file sizes bounded.
- All resulting filenames must remain compliant with the SD naming policy above.

### SS - Storage Sync

SS includes all LS behavior plus periodic synchronization to a remote endpoint.

Data mode `SS` (Storage Sync) uses all that applies to `LS` (Local Storage) but in addition
it also tries to synchronise data to a remote server, via HTTP POST requests.

The server URL should be configurable via `.env` files preferences, alongside
any API token necessary for authentication.

The synchronization should happen at regular intervals (hourly / daily) that
can also be specified via the configuration paramters of the `.env` file.

When the WiFi is not available, this data mode should continue to operate in a
LS (Local Storage) only mode, but as soon as a connection becomes available data
synchronization should take place.

Synchronization requirements:
1. Upload delta only (new data since last successful sync).
2. Persist sync checkpoint reliably (SD metadata file or EEPROM), including:
  - Last synced file path
  - Last synced byte offset or line index
3. If WiFi is unavailable:
  - Continue local logging exactly as LS mode
  - Resume sync when connectivity is restored

The storage mode should operate based on specific time intervals, in relation to
the data reading from the sensors.

### RT, LSRT, SSRT fallback behavior

For RT and combined modes:
1. RT publishes live readings via MQTT.
2. If connectivity is lost, do not drop data.
3. Fallback to local persistence behavior compatible with LS rules.
4. After reconnection, publish/resync backlog according to mode policy.


##### Tracking Synchronization

Data Synchronization should upload only the delta of changed data/files.

For that to be possible, the software should track the last data that was
successfully synchronised, and store that information reliably on the SD card
or EEPROM (whatever is best).

The syncronization point should store the file to access and the byte or line number
that data were last successfully syncronised to.

All these routing should be thoroughly unit tested to prove and assess correctness.

### RT - Real-Time

The `RT` (Real-Time) data mode will upload live data to an MTTQ Broker.

The MQTT topic should be configurable, via `.env` variables file so that the
appropriate topic can be used for the data updates.

The connection to the target MQTT Broker will be done over an available WiFi
connection that the software should manage to connect before being able to
attempt any uploads. Reconnection in case of loss should also be handled.

In case of loss of connectivity, the software should not loose data. In that case
it should fall back to the `LSRT` mode where data are locally collected and then
uploaded via MQTT. In this scenario the `LSRT` fallback mode should be in place
until data are sent over MQTT for the first time (successfully) after the WiFi and
communication with the MQTT broker is successfully reestablished.

### LSRT - Local Storage + Real-Time

The Local Storage + Real Time mode can be used to create both a local storage
of data plus a live stream via MQTT of the data.

* `LS` mode operations also apply here for handling the Local Storage.
* `RT` mode operations also apply here for handling the Real Time.

### SSRT - Storage Sync + Real-Time stream

The Storage Sync + Real-Time mode can be used to create both a local storage
of data with synchronization support plus a live stream via MQTT of the data.

* `SS` mode operations also apply here for handling the Storage Sync.
* `RT` mode operations also apply here for handling the Real Time.

# OTA Software Updates

The solution should support OTA (Over The Air) software updates mechanism. In
general, this mechanism could work either over WiFi or Bluetooth.

The functioning of this part needs to be futher clarified and defined in its
working details.

# Software Quality

All software should be organised in a modular fashon, where every part is scoped
to a well defined module.

Each module should be accompanied by a relevant suite of unit tests to prove the
software correctness. No software module is acceptable as part of the system
without tourough unit tests written for it and exercising the module with relevant
tests cases.
