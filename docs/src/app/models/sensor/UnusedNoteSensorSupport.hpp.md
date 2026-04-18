# `src/app/models/sensor/UnusedNoteSensorSupport.hpp`

## Responsibility

Declares the shared RAW note inactivity scan used by the fixed-period unused-note sensor objects.

## Public Contract

- `collectUnusedNoteEntries(...)` walks the unpacked hub, parses `.wsnhead`, and returns the note-entry payloads
  consumed by `WeeklyUnusedNote` and `MonthlyUnusedNote`.
- `noteIdsFromEntries(...)` extracts the stable `noteId` list from those entry payloads for UI callers that only need
  the filtered identifiers.
