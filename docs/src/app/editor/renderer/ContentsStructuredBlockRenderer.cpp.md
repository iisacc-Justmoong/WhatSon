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
- Source refresh no longer runs the full structured verification pass once per backend signal. The renderer now parses the
  needed backends first, pulls their cached verification snapshots, and computes the combined structured verification
  once per source refresh.
- Notes that do not contain any proprietary structured tags now skip agenda/callout backend parsing entirely and fall
  back to one plain text block plus linter-only verification.

## Why It Changed
Agenda/callout cards now render as document-owned flow blocks instead of offset-projected overlay layers. The QML host
needs one ordered block stream rather than two independent overlay lists.
