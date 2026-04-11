# `src/app/editor/renderer/ContentsStructuredBlockRenderer.cpp`

## Responsibility
Builds canonical structured render data from `.wsnbody` source text.

## Current Contract
- Still exposes `renderedAgendas` and `renderedCallouts` for legacy/fallback consumers.
- Now also exposes `renderedDocumentBlocks`, a mixed document-flow model ordered by source position.
- `renderedDocumentBlocks` interleaves:
  - plain text segments: `type=text`
  - agenda blocks: `type=agenda`
  - callout blocks: `type=callout`
  - divider tags: `type=break`
- Each non-text block carries the backend-owned source geometry so QML can rewrite RAW in place.
- `hasRenderedBlocks` now reflects any non-text block, including standalone `</break>`.

## Why It Changed
Agenda/callout cards now render as document-owned flow blocks instead of offset-projected overlay layers. The QML host
needs one ordered block stream rather than two independent overlay lists.
