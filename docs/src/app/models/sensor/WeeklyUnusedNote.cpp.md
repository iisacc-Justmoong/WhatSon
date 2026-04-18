# `src/app/models/sensor/WeeklyUnusedNote.cpp`

## Responsibility

Implements the weekly inactivity wrapper around `UnusedNoteSensorSupport`.

## Window Definition

- Captures one UTC reference time per refresh.
- Uses `referenceUtc.addDays(-7)` as the weekly cutoff.
- Leaves the shared parsing, hidden-path filtering, and payload construction to
  `UnusedNoteSensorSupport::collectUnusedNoteEntries(...)`.
