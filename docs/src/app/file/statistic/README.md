# `src/app/file/statistic`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/file/statistic`
- Child directories: 0
- Child files: 2

## Child Directories
- No child directories.

## Child Files
- `WhatSonNoteFileStatSupport.cpp`
- `WhatSonNoteFileStatSupport.hpp`

## Current Focus Areas
- `WhatSonNoteFileStatSupport` now owns reusable note-statistic support logic:
  - local body-derived counter recomputation
  - header-only `openCount` rewrites
  - hub-wide tracked-stat refresh paths such as incoming backlink counts
- The directory is no longer empty; statistic-specific logic now lives here instead of under `file/note`.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
