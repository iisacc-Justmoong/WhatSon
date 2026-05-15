# `src/app/models/file/diff/VersionLimitManager.cpp`

## Role
Implements the retention policy for `.wsnversion` snapshot lists.

## Behavior
- The maximum retained snapshot count is 100.
- When `WhatSonNoteVersionState.snapshots` exceeds that count, the helper erases the oldest records from the front of
  the vector.
- After pruning, the first retained snapshot clears `parentSnapshotId` so it does not point at a snapshot that is no
  longer owned by the store.
- If `currentSnapshotId` or `headSnapshotId` falls outside the retained list, the helper falls back to the latest
  retained snapshot ID.

## Tests
- `localNoteVersionStore_prunesSnapshotsToLatestOneHundred` captures 101 snapshots and verifies that the stored JSON
  keeps `commit:2` through `commit:101`, with `currentSnapshotId` and `headSnapshotId` pointing at the latest retained
  snapshot.
