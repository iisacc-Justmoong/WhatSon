# `src/app/models/file/hierarchy/library/LibraryAll.hpp`

## Responsibility

`LibraryAll` owns the canonical in-memory `all notes` bucket for the library domain.

## Public Contract

- `indexFromWshub(...)`: parse the mounted hub and build the canonical note vector.
- `setIndexedNotes(...)`: replace the full canonical note vector from a runtime snapshot or mutation result.
- `setSourceWshubPath(...)`: update only the source hub identity when a caller wants to preserve the current note set
  but change where future persistence should resolve from.
- `upsertNote(...)`: insert or update one note in place and return `false` for structural no-op updates.
- `removeNoteById(...)`: prune one note without replacing the whole bucket.
- `noteById(...)`: resolve one note record for mutation or projection collaborators.

## Tests
- Automated test files are not currently present in this repository.
- Regression checklist:
  - updating one note body/metadata must be representable through `upsertNote(...)` without rebuilding the full
    canonical note vector
  - deleting one note must be representable through `removeNoteById(...)`
  - a no-op upsert must return `false` so higher layers can suppress unnecessary note-list/calendar rebuilds

## Architectural Role
This header defines the stable mutation/query surface that `WhatSonLibraryIndexedState` uses to keep the canonical
library note set synchronized while avoiding a full-snapshot replacement on every local note edit.
