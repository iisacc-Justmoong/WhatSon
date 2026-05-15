# `src/app/models/file/diff/WhatSonNoteVersionSnapshotBuilder.cpp`

## Role
Implements snapshot identity, lineage, and payload assembly.

## Behavior
- Normalizes labels into filesystem- and JSON-friendly snapshot ID prefixes.
- Hashes label/header/body content to produce deterministic snapshot ID bases.
- Adds numeric suffixes when an ID collision exists in the loaded state.
- Computes snapshot diffs against the resolved parent snapshot payload.

## Retention Interaction
- This helper does not prune history.
- `VersionLimitManager` remains responsible for trimming the loaded state after a store flow appends or updates
  snapshot ownership.
