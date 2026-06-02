# `src/app/models/sensor/UnusedNoteSensorSupport.cpp`

## Responsibility

Implements the shared note inactivity scan for the sensor domain.

## Scan Rules

- Validates that the input hub is an unpacked `.wshub` directory.
- Validates the hub path and returns an empty note inactivity result while note package scanning is disabled.
- Reads `.wsnhead` as the authoritative note activity source.
- Resolves the effective activity timestamp in this order:
  - `lastOpenedAt`
  - `createdAt`
  - `lastModifiedAt`
- Sorts returned entries by `noteId` so higher-level sensors expose deterministic output.
