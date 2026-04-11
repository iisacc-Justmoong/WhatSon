# `src/app/editor/renderer`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/editor/renderer`
- Child directories: 0
- Child files: 6

## Child Directories
- No child directories.

## Child Files
- `ContentsTextHighlightRenderer.cpp`
- `ContentsTextHighlightRenderer.hpp`
- `ContentsTextFormatRenderer.cpp`
- `ContentsTextFormatRenderer.hpp`
- `ContentsPagePrintLayoutRenderer.cpp`
- `ContentsPagePrintLayoutRenderer.hpp`

## Current Notes
- `ContentsTextFormatRenderer.cpp` now treats proprietary inline source tags as the authoritative formatting basis for
  logical-selection formatting.
- Shortcut/context-menu formatting no longer depends on a transient `QTextDocument` fragment merge to decide where a
  RAW source style starts or ends.
- Page/Print paper-preview mode gating and A4 paper geometry calculations are now centralized in
  `ContentsPagePrintLayoutRenderer`, so desktop/mobile QML hosts only bind to backend state.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
