# `src/app/file/note/WhatSonLocalNoteVersionStore.hpp`

## Role
Declares the local note versioning contract that persists snapshot history in `.wsnversion`.

The header models snapshots as commit-like records and exposes the store API for read/capture/diff/checkout/rollback
operations.

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

## Tests
- This repository does not maintain in-repo automated tests for this module.
- Keep the regression checks above synchronized with `WhatSonLocalNoteVersionStore.cpp` behavior when schema fields
  change.
