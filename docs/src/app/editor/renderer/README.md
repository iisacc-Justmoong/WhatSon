# `src/app/editor/renderer`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/editor/renderer`
- Child directories: 0
- Child files: 10

## Child Directories
- No child directories.

## Child Files
- `ContentsTextHighlightRenderer.cpp`
- `ContentsTextHighlightRenderer.hpp`
- `ContentsHtmlBlockRenderPipeline.cpp`
- `ContentsHtmlBlockRenderPipeline.hpp`
- `ContentsTextFormatRenderer.cpp`
- `ContentsTextFormatRenderer.hpp`
- `ContentsStructuredBlockRenderer.cpp`
- `ContentsStructuredBlockRenderer.hpp`
- `ContentsPagePrintLayoutRenderer.cpp`
- `ContentsPagePrintLayoutRenderer.hpp`

## Current Notes
- `ContentsHtmlBlockRenderPipeline.cpp` is now the explicit editor HTML pipeline:
  parser blocks -> HTML tokens -> normalized HTML blocks -> final editor HTML document.
- `ContentsTextFormatRenderer.cpp` now treats proprietary inline source tags as the authoritative formatting basis for
  logical-selection formatting.
- `ContentsTextFormatRenderer.cpp` now also republishes that pipeline's intermediate payloads (`htmlTokens`,
  `normalizedHtmlBlocks`, `htmlOverlayVisible`) so block-level QML renderers can consume one stable decision instead of
  inferring semantic presentation only from regexes over raw source text.
- `ContentsStructuredBlockRenderer.cpp` now consumes `parser/ContentsWsnBodyBlockParser` as its single `.wsnbody`
  read-path source and republishes that parser result to QML.
- Agenda/callout compatibility lists still exist, but they are now side projections over the same unified parser pass
  rather than separate read-side backend merges.
- Shortcut/context-menu formatting no longer depends on a transient `QTextDocument` fragment merge to decide where a
  RAW source style starts or ends.
- Page/Print paper-preview mode gating and A4 paper geometry calculations are now centralized in
  `ContentsPagePrintLayoutRenderer`, so desktop/mobile QML hosts only bind to backend state.
- `ContentsStructuredBlockRenderer.cpp` and `ContentsTextFormatRenderer.cpp` now also emit verbose editor trace events
  for constructor/destructor turns, source binding changes, synchronous/background render refresh passes, placeholder
  publication, and final payload application so the read-side render pipeline can be traced alongside QML host updates.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
