# `src/app/models/file/diff/WhatSonNoteVersionSnapshotBuilder.hpp`

## Role
Declares snapshot lookup and construction behavior for local note version history.

## Contract
- Finds snapshots by ID in a loaded `WhatSonNoteVersionState`.
- Resolves the parent snapshot for capture from current, head, or latest retained history.
- Builds a complete `WhatSonNoteVersionSnapshot` with identity, lineage, payload text, timestamp, and diff metadata.

## Boundary
- Does not read or write files.
- Does not serialize `.wsnversion` JSON.
- Delegates diff construction to `WhatSonNoteVersionDiffBuilder`.
