# `src/app/file/sync`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/file/sync`
- Child directories: 0
- Child files: 4

## Child Directories
- No child directories.

## Child Files
- `ContentsEditorIdleSyncController.cpp`
- `ContentsEditorIdleSyncController.hpp`
- `WhatSonHubSyncController.cpp`
- `WhatSonHubSyncController.hpp`

## Current Notes

- `ContentsEditorIdleSyncController` owns the editor-side buffered fetch-sync boundary under the `file/sync` domain.
- `WhatSonHubSyncController` was moved from `src/app/sync` into this directory so sync responsibilities are now
  fully consolidated under `file/sync`.
- Runtime hub synchronization and editor idle synchronization now share the same domain root while preserving their
  existing responsibilities.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - Startup/runtime wiring must still resolve `WhatSonHubSyncController` through the new `file/sync` include path.
  - Hub watcher debounce and local-mutation acknowledge flow must remain unchanged after the directory move.
  - Editor idle sync (`ContentsEditorIdleSyncController`) behavior must remain unchanged.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
