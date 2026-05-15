# `src/app/models/file/diff/WhatSonNoteVersionFileGateway.hpp`

## Role
Declares the local note version file gateway.

## Contract
- Resolves note ID plus `.wsnversion`, `.wsnhead`, and `.wsnbody` paths from `WhatSonLocalNoteDocument`.
- Reads and writes UTF-8 text files.
- Materializes a missing `.wsnversion` document from serialized empty-state text supplied by the codec.

## Boundary
- The gateway owns filesystem concerns only.
- It does not serialize version state, build snapshots, compute diffs, or apply retention policy.
