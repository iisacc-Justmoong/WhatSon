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
- `ContentsHtmlBlockRenderPipeline.cpp`
- `ContentsHtmlBlockRenderPipeline.hpp`
- `ContentsStructuredBlockRenderer.cpp`
- `ContentsStructuredBlockRenderer.hpp`

## Current Notes
- `ContentsHtmlBlockRenderPipeline.cpp` is now the explicit editor HTML pipeline:
  parser blocks -> HTML tokens -> normalized HTML blocks -> final editor HTML document.
- `ContentsStructuredBlockRenderer.cpp` now consumes `parser/ContentsWsnBodyBlockParser` as its single `.wsnbody`
  read-path source and republishes that parser result to QML.
- Agenda/callout compatibility lists still exist, but they are now side projections over the same unified parser pass
  rather than separate read-side backend merges.
- Shortcut/context-menu formatting no longer depends on a transient `QTextDocument` fragment merge to decide where a
  RAW source style starts or ends.
- Page/print paper-surface helpers were moved out to `src/app/models/display/paper` and
  `src/app/models/display/paper/print`, leaving this directory focused on renderer-only concerns.
- `ContentsStructuredBlockRenderer.cpp` still emits verbose editor trace events for constructor/destructor turns,
  source binding changes, and projection refresh passes so the read-side renderer path can be traced alongside QML host
  updates.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
