# `src/app/models/file/note/local`

## Scope
- Owns the local note package document shape and file-store API for reading and mutating `.wsnote` packages.
- Keeps concrete package IO behind a note-domain object rather than shared generic IO helpers.

## Files
- `WhatSonLocalNoteDocument.hpp`
- `WhatSonLocalNoteFileStore.*`

## Boundary
- Provides package read/write operations used by body, header, folder, hub, and editor-domain collaborators.
- Must not decide view routing, hierarchy controller behavior, or editor input policy.
