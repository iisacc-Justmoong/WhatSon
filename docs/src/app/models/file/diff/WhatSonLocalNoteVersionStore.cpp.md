# `src/app/models/file/diff/WhatSonLocalNoteVersionStore.cpp`

## Role
Implements the facade/orchestrator for `.wsnversion` snapshot flows for one local note package.

The store keeps the public API stable while delegating focused responsibilities to dedicated collaborators:

- `WhatSonNoteVersionFileGateway`: note path resolution, UTF-8 reads/writes, and empty version file materialization.
- `WhatSonNoteVersionStateCodec`: `.wsnversion` JSON serialization and parsing.
- `WhatSonNoteVersionSnapshotBuilder`: snapshot lookup, parent resolution, ID generation, and snapshot assembly.
- `WhatSonNoteVersionDiffBuilder`: diff segment and unified patch construction.
- `VersionLimitManager`: retention pruning before persistence.

## Core Flow
1. `loadState(...)`
   - Normalizes the `.wsnversion` path.
   - Uses `WhatSonNoteVersionFileGateway` to read the file.
   - Uses `WhatSonNoteVersionStateCodec` to parse persisted snapshots into `WhatSonNoteVersionState`.
2. `captureSnapshot(...)`
   - Resolves note/version/header/body paths through `WhatSonNoteVersionFileGateway`.
   - Materializes an empty `.wsnversion` document if missing.
   - Reads current `.wsnhead` and `.wsnbody` as the working tree snapshot.
   - Builds a new snapshot through `WhatSonNoteVersionSnapshotBuilder`.
   - Persists commit metadata (`commitModifiedCount`) and precomputed diff payload (`headerDiff`, `bodyDiff`).
   - Applies `VersionLimitManager` before writing so only the latest 100 snapshots remain in `.wsnversion`.
3. `diffSnapshots(...)`
   - Resolves two snapshots by ID through `WhatSonNoteVersionSnapshotBuilder`.
   - Computes diff segments with `WhatSonNoteVersionDiffBuilder`.
4. `checkoutSnapshot(...)` / `rollbackToSnapshot(...)`
   - Writes snapshot payload back to working files.
   - Updates `currentSnapshotId`.
   - Rollback also records a new rollback snapshot in history.

## Unified Patch Contract
- Diff segments now include `unifiedPatch`.
- Diff segments also include `generatedAtUtc`, captured when `WhatSonNoteVersionDiffBuilder` creates the segment.
- Patch text uses a git-like unified format:
  - `--- a/<label>`
  - `+++ b/<label>`
  - `@@ -x,n +y,m @@`
  - removed (`-`) and inserted (`+`) lines
- Body diffs are computed from `bodyDocumentText` so persisted `.wsnbody` deltas are trackable as file-level changes.
- Timestamp text uses `Qt::ISODateWithMs` UTC format so persisted `.wsnversion` diff records can be sorted or audited
  independently from the snapshot timestamp.

## Error Handling
- Any parse/read/write failure returns `false` and surfaces a human-readable message through `errorMessage`.
- Capture/checkpoint operations are fail-fast; no best-effort partial append is allowed inside `.wsnversion`.
- Snapshot retention is deterministic: once a write would exceed 100 snapshots, the oldest snapshots are removed from
  the front of the list and the first retained snapshot starts a new retained parent chain.

## Regression Checks
- The store source must remain a facade and include the dedicated responsibility objects instead of re-owning
  `QSaveFile`, `QJsonDocument`, cryptographic hash, or regular-expression snapshot ID logic.
- Capturing a snapshot with no parent must still produce valid diff metadata (empty-base insertion patch).
- Capturing with a parent must compute diffs against the parent snapshot payload.
- Loading legacy snapshots with missing diff keys must keep defaults and remain readable.
- Rollback snapshots must persist a new commit record with parent/source lineage.
- Capturing 101 snapshots must persist exactly 100 records, retaining `commit:2` through `commit:101` and dropping the
  original oldest snapshot.

## Tests
- `test/cpp/suites/note_version_store_tests.cpp` verifies that version responsibilities are split into dedicated
  objects and that the store no longer owns low-level file IO, JSON codec, hash, or regex dependencies directly.
- `test/cpp/suites/note_version_store_tests.cpp` verifies that a note update which advances `modifiedCount` appends a
  `.wsnversion` snapshot with the expected `commitModifiedCount`, payload text, and unified patch metadata.
- The same suite verifies that snapshot retention keeps only the newest 100 records.
- Keep these regression checks updated when snapshot schema or patch formatting changes.
