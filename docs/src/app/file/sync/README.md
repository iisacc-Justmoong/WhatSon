# `src/app/file/sync`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/file/sync`
- Child directories: 0
- Child files: 2

## Child Directories
- No child directories.

## Child Files
- `ContentsEditorIdleSyncController.cpp`
- `ContentsEditorIdleSyncController.hpp`

## Current Notes

- `ContentsEditorIdleSyncController` now owns the editor idle gate under the `file/sync` domain.
- QML/editor code stages body snapshots only; it no longer owns the `1000ms` inactivity decision locally.
- The same controller also handles note-exit flush promotion, but actual `.wsnote` persistence still remains
  asynchronous because it forwards into `file/note/ContentsNoteManagementCoordinator`.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
