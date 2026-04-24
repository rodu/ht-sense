# ADR 003 — Data Mode Offline Fallback: Network Modes Fall Back to LS, Not NC

**Date:** 2026-04-24
**Status:** Accepted

---

## Context

Several data modes require a network connection:

- `RT` — publishes live readings via MQTT.
- `SS` — stores locally and periodically syncs to a remote HTTP endpoint.
- `LSRT` — stores locally and streams live via MQTT.
- `SSRT` — stores locally, syncs remotely, and streams live via MQTT.

When WiFi or MQTT connectivity is lost, the firmware must decide what to do
with new sensor readings. The options are:

1. **Drop readings silently** (stay in the configured mode, skip network calls).
2. **Fall back to NC** (no collection — discard everything).
3. **Fall back to LS** (local storage — keep collecting to SD card).
4. **Fall back to the local-only subset of the mode** (same as option 3 for
   all current modes).

---

## Decision

All modes that require a network connection **fall back to LS** when offline.

The fallback table implemented in `fallbackModeWhenOffline()`:

| Mode | Offline fallback | Reason |
|---|---|---|
| NC | NC | No network dependency; unchanged |
| LS | LS | Already local-only; unchanged |
| SS | LS | Continue collecting; resume sync on reconnect |
| RT | LS | Buffer locally to avoid data loss |
| LSRT | LS | Drop MQTT stream; keep local part |
| SSRT | LS | Drop MQTT stream and sync; keep local part |
| UNKNOWN | NC | Cannot determine intent; safest is to collect nothing |

---

## Rationale

**Why not stay in the configured mode and just skip network calls?**

`RT` mode has no local storage path. If it simply skips MQTT publishes while
offline, readings are silently lost. This violates the project requirement:
_"In case of loss of connectivity, the software should not lose data."_

**Why not fall back to NC?**

NC discards readings. If the user configured `SS` or `RT`, they intend to keep
data. Falling back to NC would cause data loss during any connectivity outage,
which is the opposite of the intended behaviour.

**Why LS specifically?**

`LS` is the simplest mode that reliably preserves data locally. It has no
network dependency, so it is guaranteed to work regardless of WiFi state.
When connectivity is restored, `SS`/`SSRT` can resume syncing the data that was
accumulated during the outage (this is the delta-sync design of those modes).

**Why does UNKNOWN fall back to NC rather than LS?**

`UNKNOWN` means the `DATA_MODE` value in `.env` could not be parsed. The user's
intent is not known. In this case, collecting data to SD under an unknown
configuration risks filling the SD card unexpectedly. NC is the safer default:
it does nothing harmful, and a warning is logged so the operator can fix the
configuration.

---

## Consequences

- The `config_parser` and mode-switching logic must call `fallbackModeWhenOffline`
  when a network connectivity change is detected, not when entering the loop.
- When connectivity is restored, the system must switch back to the originally
  configured mode (not stay permanently in LS). The active mode variable in
  `main.cpp` tracks the current effective mode; the configured mode from `.env`
  must be kept separately so it can be restored.
- `RT` mode's offline buffer (LS fallback) means the SD card must be available
  even in `RT` mode during outages. The hardware initialisation must account for
  this — SD initialisation cannot be conditional on the initial configured mode.
