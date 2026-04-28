# `src/app/models/editor/format/ContentsInlineStyleOverlayRenderer.hpp`

## Responsibility

Declares the block-local inline-style overlay renderer used by structured text delegates.

## Public Contract

- `sourceText`
  The authoritative block-local RAW slice that may contain inline style tags.
- `editorSurfaceHtml`
  Read-side HTML overlay for the plain-text `TextEdit` surface.
- `htmlOverlayVisible`
  Mirrors whether the current block-local RAW slice produces any formatted HTML overlay worth painting.
- `paperPaletteEnabled`
  Recolors the overlay for page/print surfaces.
- `requestRenderRefresh()`
  Forwards an explicit refresh request when the host needs one more immediate recompute turn.

## Boundary

This type exists so block-local inline formatting display does not need the wider document renderer contract. It does
not own typing mutation, preview HTML, or document-wide render payloads.
