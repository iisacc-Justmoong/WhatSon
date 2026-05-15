# `src/app/models/file/diff/WhatSonLocalNoteVersionStore.hpp`

## Role
Declares the local note versioning contract that persists snapshot history in `.wsnversion`.

The header models snapshots as commit-like records and exposes the store API for read/capture/diff/checkout/rollback
operations.

Implementation details are intentionally split out of the store source. File IO, JSON codec, diff construction, and
snapshot assembly are handled by dedicated helper objects in the same module while this header remains the public
contract consumed by the note file store and regression tests.

## Snapshot Schema Contract
- `WhatSonNoteVersionSnapshot` represents one persisted point-in-time state.
- Snapshot identity and lineage:
  - `snapshotId`
  - `parentSnapshotId`
  - `sourceSnapshotId` (used by rollback lineage)
- Commit mapping:
  - `commitModifiedCount` stores the `fileStat.modifiedCount` value that triggered this capture.
- Payload:
  - `headerText`
  - `bodyDocumentText`
  - `bodyPlainText`
- Precomputed patch payload:
  - `headerDiff`
  - `bodyDiff`
- `WhatSonNoteVersionDiffSegment` includes:
  - prefix/suffix lengths
  - removed/inserted text blocks
  - `unifiedPatch` (`git diff`-style unified text)

## Request Types
- `CaptureRequest` includes `commitModifiedCount` so callers can bind snapshot capture to a specific note commit.
- `DiffRequest`, `CheckoutRequest`, and `RollbackRequest` provide explicit snapshot IDs and keep mutation flow
  deterministic.

## Regression Checks
- Backward compatibility: loading old `.wsnversion` files without `commitModifiedCount` or `*Diff` fields must still
  succeed with defaults.
- New captures must persist both raw snapshot payload and unified patch metadata.
- Persisted snapshot history must be capped at the latest 100 snapshots; records beyond that cap are pruned oldest
  first by `VersionLimitManager` during write flows.

## Tests
- `test/cpp/suites/note_version_store_tests.cpp` covers the commit snapshot created through
  `WhatSonLocalNoteFileStore::updateNote(...)`.
- The same suite covers the 100-snapshot retention cap.
- The same suite also guards the responsibility split so future changes do not collapse IO, JSON, diff, and snapshot
  construction back into `WhatSonLocalNoteVersionStore.cpp`.
- Keep the regression checks above synchronized with `WhatSonLocalNoteVersionStore.cpp` behavior when schema fields
  change.
