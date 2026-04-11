# `src/app/file/diff/WhatSonLocalNoteVersionStore.cpp`

## Role
Implements `.wsnversion` persistence and snapshot lifecycle for one local note package.

## Core Flow
1. `ensureState(...)`
   - Resolves note ID and `.wsnversion` path.
   - Materializes an empty version document if missing.
   - Parses persisted snapshots into `WhatSonNoteVersionState`.
2. `captureSnapshot(...)`
   - Reads current `.wsnhead` and `.wsnbody` as the working tree snapshot.
   - Appends a new snapshot with parent linkage.
   - Persists commit metadata (`commitModifiedCount`) and precomputed diff payload (`headerDiff`, `bodyDiff`).
3. `diffSnapshots(...)`
   - Resolves two snapshots by ID.
   - Computes diff segments with unified patch text for header/body.
4. `checkoutSnapshot(...)` / `rollbackToSnapshot(...)`
   - Writes snapshot payload back to working files.
   - Updates `currentSnapshotId`.
   - Rollback also records a new rollback snapshot in history.

## Unified Patch Contract
- Diff segments now include `unifiedPatch`.
- Patch text uses a git-like unified format:
  - `--- a/<label>`
  - `+++ b/<label>`
  - `@@ -x,n +y,m @@`
  - removed (`-`) and inserted (`+`) lines
- Body diffs are computed from `bodyDocumentText` so persisted `.wsnbody` deltas are trackable as file-level changes.

## Error Handling
- Any parse/read/write failure returns `false` and surfaces a human-readable message through `errorMessage`.
- Capture/checkpoint operations are fail-fast; no best-effort partial append is allowed inside `.wsnversion`.

## Regression Checks
- Capturing a snapshot with no parent must still produce valid diff metadata (empty-base insertion patch).
- Capturing with a parent must compute diffs against the parent snapshot payload.
- Loading legacy snapshots with missing diff keys must keep defaults and remain readable.
- Rollback snapshots must persist a new commit record with parent/source lineage.

## Tests
- In-repo automated tests are currently absent by repository policy.
- Keep these regression checks updated when snapshot schema or patch formatting changes.
