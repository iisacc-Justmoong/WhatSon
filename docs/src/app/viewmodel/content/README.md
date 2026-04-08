# `src/app/viewmodel/content`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/viewmodel/content`
- Child directories: 0
- Child files: 6

## Child Directories
- No child directories.

## Child Files
- `ContentsEditorSelectionBridge.cpp`
- `ContentsEditorSelectionBridge.hpp`
- `ContentsGutterMarkerBridge.cpp`
- `ContentsGutterMarkerBridge.hpp`
- `ContentsLogicalTextBridge.cpp`
- `ContentsLogicalTextBridge.hpp`

## Current Notes

- `ContentsEditorSelectionBridge` now owns the asynchronous direct `.wsnote` save queue for editor body writes.
- The editor/UI path only enqueues `{noteId, noteDirectoryPath, bodyText}`; the worker thread re-reads and updates the
  actual `.wsnote` package.
- Note-selection changes now reuse the same `{noteId, noteDirectoryPath}` metadata session and no longer trigger a
  hub-wide `.wsnbody` stat refresh just to bump `openCount`.
- The bridge only applies persisted body state and requests tracked-stat refresh after background completion returns to
  the main thread.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
