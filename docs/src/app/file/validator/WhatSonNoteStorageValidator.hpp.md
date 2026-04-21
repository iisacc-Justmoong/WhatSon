# `src/app/models/file/validator/WhatSonNoteStorageValidator.hpp`

## Role
`WhatSonNoteStorageValidator` resolves materialized note storage paths and now owns `.wsnote` package normalization.

## Public API
- `resolveExistingNoteHeaderPath(const LibraryNoteRecord&)`
  - Resolves a usable `.wsnhead` path from direct record fields or from a materialized note directory.
- `resolveExistingNoteDirectoryPath(const LibraryNoteRecord&)`
  - Resolves the backing `.wsnote` directory from directory/header hints.
- `hasMaterializedStorage(const LibraryNoteRecord&)`
  - Returns whether either header or note directory is physically present.
- `normalizeWsnotePackage(const LibraryNoteRecord&, QString* errorMessage)`
  - Enforces the note package contract in-place and reports filesystem errors through `errorMessage`.

## `.wsnote` Contract
- The validator treats the following files as the only allowed package payload:
  - `<note-stem>.wsnhead`
  - `<note-stem>.wsnbody`
  - `<note-stem>.wsnversion`
  - `<note-stem>.wsnpaint`
- Any extra sidecar file (for example `*.wsnlink`, `*.wsnhistory`, ad-hoc payloads) is considered package pollution and removed.
- Any subdirectory (including hidden folders such as `.meta` and legacy folders such as `attachments/`) is removed recursively.
- Missing required files are materialized:
  - `.wsnhead` and `.wsnbody` get empty XML defaults.
  - `.wsnversion` and `.wsnpaint` get schema defaults.

## Dependency Notes
- Uses `WhatSonSystemIoGateway` for read/write/delete operations and error propagation.
- Exposes no QObject surface; this is a pure filesystem validation utility used by runtime validators.
