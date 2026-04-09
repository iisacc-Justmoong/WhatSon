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

- `ContentsEditorIdleSyncController` now owns the editor-side buffered fetch-sync boundary under the `file/sync`
  domain.
- QML/editor code stages the latest body snapshot per note only; it no longer owns any "save on this exact idle turn"
  decision locally.
- The same controller also handles best-effort immediate fetch requests for lifecycle edges, but actual `.wsnote`
  persistence still remains asynchronous because it forwards into `file/note/ContentsNoteManagementCoordinator`.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
