# `src/app/models/file/diff/WhatSonNoteVersionFileGateway.cpp`

## Role
Implements path resolution and UTF-8 file access for local note versioning.

## Behavior
- Uses direct document paths when present.
- Falls back to the note directory stem for `.wsnversion`, `.wsnhead`, and `.wsnbody` paths.
- Uses `QSaveFile` for atomic UTF-8 writes.
- Creates parent directories before writing.

## Error Handling
- Read/write/materialization failures return `false` and surface a human-readable `errorMessage`.
- Empty version paths are rejected before materialization.
