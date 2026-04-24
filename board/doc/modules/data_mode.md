# Module: data_mode

**Location:** `lib/data_mode/`
**Files:** `data_mode.h`, `data_mode.cpp`
**Environment:** native + uno_r4_wifi (no Arduino dependency)
**Tests:** `test/test_data_mode/test_main.cpp`

---

## Purpose

Parses the `DATA_MODE` value from the `.env` configuration file and provides
helper predicates that let the rest of the firmware ask capability questions
("does this mode write to SD?") rather than branching on enum values directly.

It also defines the offline fallback policy: when network connectivity is lost,
which mode should the system degrade to?

---

## Data Modes

| Value | Name | Description |
|-------|------|-------------|
| `NC` | No Collection | Run normally but discard every reading |
| `LS` | Local Storage | Persist readings to SD card |
| `SS` | Storage Sync | LS + periodic upload to remote endpoint |
| `RT` | Real-Time | Publish live readings via MQTT |
| `LSRT` | Local Storage + Real-Time | LS and RT simultaneously |
| `SSRT` | Storage Sync + Real-Time | SS and RT simultaneously |
| `UNKNOWN` | — | Parse failed; treat as NC |

---

## API

```cpp
enum class DataMode : uint8_t {
    NC      = 0,
    LS      = 1,
    SS      = 2,
    RT      = 3,
    LSRT    = 4,
    SSRT    = 5,
    UNKNOWN = 255
};

// Parse a DATA_MODE string. Case-sensitive, exact match.
// Returns UNKNOWN for null, empty, or unrecognised input.
DataMode parseDataMode(const char *str);

// Capability predicates
bool supportsLocalStorage(DataMode mode);  // true for LS, SS, LSRT, SSRT
bool supportsRealtime(DataMode mode);      // true for RT, LSRT, SSRT
bool supportsSync(DataMode mode);          // true for SS, SSRT

// Offline fallback — see ADR 003
DataMode fallbackModeWhenOffline(DataMode mode);

// Canonical string ("NC", "LS", etc. — "UNKNOWN" for DataMode::UNKNOWN)
const char *dataModeToString(DataMode mode);
```

---

## Parsing Rules

- Input must be **exact and uppercase**: `"LS"` parses; `"ls"` and `"Ls"` do not.
- Whitespace is **not trimmed** by this module. Trimming is the `config_parser`
  module's responsibility.
- `nullptr` and empty string both return `DataMode::UNKNOWN`.

---

## Offline Fallback

`fallbackModeWhenOffline` returns the mode to use when WiFi or MQTT
connectivity is lost:

| Active mode | Fallback | Reason |
|-------------|----------|--------|
| NC | NC | No network dependency |
| LS | LS | Already local-only |
| SS | LS | Stop syncing, keep collecting |
| RT | LS | Buffer locally to avoid data loss |
| LSRT | LS | Drop MQTT stream, keep local part |
| SSRT | LS | Drop MQTT stream and sync, keep local part |
| UNKNOWN | NC | Safest option: collect nothing |

The key principle: **never fall back to NC when there is data at risk**.
RT falls back to LS (not NC) specifically to avoid silently dropping readings
during a connectivity outage. See
[decisions/003-data-mode-fallback.md](../decisions/003-data-mode-fallback.md).

---

## Usage

```cpp
#include "data_mode.h"

// Parsing from config
DataMode mode = parseDataMode(config.dataMode);
if (mode == DataMode::UNKNOWN) {
    Log.warn("Invalid DATA_MODE; defaulting to NC");
    mode = DataMode::NC;
}

// Capability checks
if (supportsLocalStorage(mode)) { /* init SD card */ }
if (supportsRealtime(mode))     { /* init MQTT */    }
if (supportsSync(mode))         { /* init sync timer */ }

// Offline degradation
if (!wifiConnected) {
    mode = fallbackModeWhenOffline(mode);
}

// Logging
Log.info(dataModeToString(mode));
```

---

## Test Coverage

46 Unity tests across 5 groups:

| Group | Cases |
|---|---|
| `parseDataMode` valid | All 6 named modes |
| `parseDataMode` invalid | null, empty, lowercase, mixed case, whitespace, garbage |
| `supportsLocalStorage` | All 6 modes + UNKNOWN |
| `supportsRealtime` | All 6 modes + UNKNOWN |
| `supportsSync` | All 6 modes + UNKNOWN |
| `fallbackModeWhenOffline` | All 6 modes + UNKNOWN |
| Round-trip | `dataModeToString` → `parseDataMode` for all named modes |
