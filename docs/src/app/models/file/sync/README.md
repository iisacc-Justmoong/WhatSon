# `src/app/models/file/sync`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/file/sync`
- Child directories: 0
- Child files: 2

## Child Directories
- No child directories.

## Child Files
- `WhatSonHubSyncController.cpp`
- `WhatSonHubSyncController.hpp`

## Current Notes

- `WhatSonHubSyncController` was moved from `src/app/sync` into this directory so sync responsibilities are now
  fully consolidated under `file/sync`.
- Runtime hub synchronization remains the responsibility of this directory.
- Note editor persistence is no longer part of `file/sync`; it is owned by
  `src/app/models/editor/persistence/ContentsEditorPersistenceController`.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - Startup/runtime wiring must still resolve `WhatSonHubSyncController` through the new `file/sync` include path.
  - Hub watcher debounce and local-mutation acknowledge flow must remain unchanged after the directory move.
  - Editor persistence tests must continue to reject any new dependency on the removed editor-persistence path under
    `src/app/models/file/sync`.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
