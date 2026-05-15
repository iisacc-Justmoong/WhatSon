# `src/app/models/file/sync/WhatSonHubSyncObservationBuilder.cpp`

## Role
Implements mounted `.wshub` observation for runtime synchronization.

## Behavior
- Recursively walks the hub with `QDirIterator`.
- Builds signature records from relative path, entry type, size, and last-modified timestamp.
- Hashes sorted signature records with SHA-256.
- Returns normalized directory watch paths from the same traversal pass.
- Ignores `.whatson` and its descendants so app-private bookkeeping does not trigger runtime reloads.

## Tests
- `hubSyncObservationBuilder_ignoresPrivateWhatSonBookkeeping` verifies that `.whatson` changes do not alter the
  observed signature while visible hub content changes do.
