# `src/app/editor`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/editor`
- Child directories: 2
- Child files: 0

## Child Directories
- `painter`
- `renderer`

## Child Files
- No direct source files.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Current Notes
- The current UI-thread hotspot priority is note-open behavior, not steady-state typing.
- Note-open reconcile and structured-tag correction now route file I/O through worker-thread queues before the result
  is mirrored back onto the main thread.
- Structured-flow block edits now also stop short of rebuilding the hidden legacy editor presentation on every
  keystroke; the fallback inline editor is only repopulated when the note exits structured mode.
- Structured-flow focus restoration now targets one reparsed block per request instead of fanning out through the whole
  document tree, which reduces per-mutation main-thread work on longer structured notes.
- The repository still does not provide an in-repo automated editor test suite; regression coverage for this area is
  documented through per-file notes rather than executable tests.
