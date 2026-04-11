# `src/app/file/diff`

## Responsibility
Owns note snapshot diff/version logic centered on `.wsnversion` persistence and snapshot comparison.

## Scope
- Source directory: `src/app/file/diff`
- Child directories: none
- Child files: 2

## Child Files
- `WhatSonLocalNoteVersionStore.hpp`
- `WhatSonLocalNoteVersionStore.cpp`

## Architectural Notes
- Diff/version code was consolidated from `src/app/file/note` into this domain.
- `file/note/WhatSonLocalNoteFileStore` remains the note package orchestrator and delegates version snapshot/diff
  persistence to this module.

## Dependency Direction
- Depends on note document/header/body types under `src/app/file/note`.
- Consumed by note store update flow for capture/diff/checkout/rollback operations.
