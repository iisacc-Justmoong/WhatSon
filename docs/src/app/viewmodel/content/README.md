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

- `ContentsEditorSelectionBridge` no longer owns the asynchronous direct `.wsnote` save queue itself.
- The editor/UI path now stages body-write intent into `file/sync/ContentsEditorIdleSyncController`, which owns the
  worker-thread `1000ms` idle gate and explicit note-exit flush promotion.
- Downstream note-management work still lives under the `file/note` domain in `ContentsNoteManagementCoordinator`,
  which performs actual persistence, open-count maintenance, and tracked-stat follow-up later.
- Note-selection changes now reuse the same `{noteId, noteDirectoryPath}` metadata session and no longer trigger a
  hub-wide `.wsnbody` stat refresh just to bump `openCount`.
- The coordinator now applies persisted body state and schedules tracked-stat refresh after background completion returns
  to the main thread.
- `ContentsLogicalTextBridge` now also exports its cached logical-to-source offset table in one QML call so the editor
  typing hot path can reseed once per presentation commit and avoid whole-note bridge regeneration on every typed
  character.
- `ContentsLogicalTextBridge` now also accepts incremental live typing adoption from QML, so the bridge no longer needs
  to rebuild line starts and logical/source offset tables from the whole note after every committed character.
- `ContentsLogicalTextBridge` now normalizes Qt container `size()` values through a bounded integer helper before
  reserve/export paths, which keeps Apple libc++ from failing mixed `int` / `qsizetype` template deduction in the
  live-typing bridge code.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
