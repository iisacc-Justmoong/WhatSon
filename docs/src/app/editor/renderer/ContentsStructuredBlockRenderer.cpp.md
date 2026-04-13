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
  - resource blocks: `type=resource`
  - divider tags: `type=break`
- Each non-text block carries the backend-owned source geometry so QML can rewrite RAW in place.
- Resource blocks also carry a stable `resourceIndex` plus the canonical tag attributes (`resourceId`,
  `resourcePath`, `resourceType`, `resourceFormat`) so the structured QML host can match the block back to
  `ContentsBodyResourceRenderer`'s resolved asset entry and paint the real package payload instead of the
  `.wsresource` directory path itself.
- `hasRenderedBlocks` now reflects any non-text block, including standalone `</break>` and `<resource ... />`.
- `hasNonResourceRenderedBlocks` now gives QML the narrower activation signal used by the editor host:
  resource-only notes do not force a host swap from the legacy inline editor into `ContentsStructuredDocumentFlow`,
  but notes containing agenda/callout/break blocks still do.
- Source refresh no longer runs the full structured verification pass once per backend signal. The renderer now parses the
  needed backends first, pulls their cached verification snapshots, and computes the combined structured verification
  once per source refresh.
- Notes that do not contain any proprietary structured tags now skip agenda/callout backend parsing entirely and fall
  back to one plain text block plus linter-only verification.
- When the host enables `backgroundRefreshEnabled`, notes that may contain agenda/callout/resource/break blocks now
  publish a cheap single-text-block placeholder immediately and compute the expensive structured render snapshot on a
  worker thread.
- Async render results are sequence-checked before apply so stale note-open parses cannot overwrite newer source text
  after selection changes or live edits.
- Both the async apply path and the placeholder publish path now compare render payload deltas first, then emit
  `renderedBlocksChanged()` from that distinct delta flag. This keeps the signal contract intact without shadowing the
  signal name in local state.

## Why It Changed
Agenda/callout/resource cards now render as document-owned flow blocks instead of offset-projected overlay layers. The
QML host needs one ordered block stream rather than multiple independent overlay lists. The latest regression also
showed that running structured parsing plus canonicalization synchronously during note-open could block the UI thread
long enough to surface as `Not responding`.
