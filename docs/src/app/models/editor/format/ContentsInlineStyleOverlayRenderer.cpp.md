# `src/app/models/editor/format/ContentsInlineStyleOverlayRenderer.cpp`

## Responsibility

Implements the narrow block-local inline-style overlay renderer.

## Current Behavior

- Owns one internal `ContentsTextFormatRenderer` instance as a private collaborator.
- Republishes only the subset needed by `ContentsDocumentTextBlock.qml`:
  - `sourceText`
  - `editorSurfaceHtml`
  - `htmlOverlayVisible`
  - `paperPaletteEnabled`
- Leaves preview HTML, normalized HTML token payloads, and source mutation APIs outside this block-local surface.

## Boundary

This file is an isolation adapter. It keeps structured text blocks from depending directly on the full document
renderer interface.
