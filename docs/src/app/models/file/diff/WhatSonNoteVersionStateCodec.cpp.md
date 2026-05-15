# `src/app/models/file/diff/WhatSonNoteVersionStateCodec.cpp`

## Role
Implements `.wsnversion` JSON serialization and parsing.

## Schema
- Root fields:
  - `version`
  - `schema`
  - `noteId`
  - `currentSnapshotId`
  - `headSnapshotId`
  - `snapshots`
- Snapshot fields include identity, lineage, commit metadata, header/body payload text, plain body text, and precomputed
  header/body diff segments.

## Compatibility
- Missing snapshot diff or commit fields parse with default values from the public snapshot structs.
- Invalid JSON returns `false` and includes the `.wsnversion` path in `errorMessage`.
