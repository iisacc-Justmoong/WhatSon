# `src/app/models/file/diff/VersionLimitManager.h`

## Role
Declares the snapshot retention helper for local note version history.

## Contract
- `maximumSnapshotCount()` returns the fixed persisted history cap: 100 snapshots.
- `pruneOldestSnapshots(...)` mutates a `WhatSonNoteVersionState` in memory before it is serialized.
- The helper is intentionally small and stateless; it does not read or write files directly.
- Retention is newest-first by survival: when the cap is exceeded, snapshots are removed from the beginning of the
  persisted list.

## Tests
- `test/cpp/suites/note_version_store_tests.cpp` verifies the helper through `WhatSonLocalNoteVersionStore` by capturing
  101 snapshots and asserting that only the latest 100 are written.
