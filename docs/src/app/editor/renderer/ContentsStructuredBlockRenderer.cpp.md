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
- The renderer now gathers supported top-level body tags through one shared explicit-block collector instead of
  splicing separate per-tag block streams together ad hoc.
  In other words, `paragraph`, `title`, `resource`, `agenda`, `callout`, and `break` all enter the editor-facing
  document flow through the same block-span path, with specialized payload fields layered on afterward where needed.
- Canonical semantic body elements such as `paragraph`, `p`, `title`, `subTitle`, `eventTitle`, and
  `eventDescription` now materialize as explicit block spans whose `type` is the canonical tag name itself
  (for example `type=paragraph`, `type=title`, `type=subtitle`, `type=eventtitle`).
  Resource-bearing notes therefore keep their authored prose blocks visible in the structured document flow instead of
  relying on one monolithic pre-resource text fragment.
- Each explicit block, including semantic text-tag blocks, carries the backend-owned source geometry so QML can rewrite
  RAW in place.
- Explicit blocks now also carry one normalized `tagName` alongside `type`, so downstream QML sees the same
  document-block identity contract regardless of which supported tag family produced the span.
- Resource blocks also carry a stable `resourceIndex` plus the canonical tag attributes (`resourceId`,
  `resourcePath`, `resourceType`, `resourceFormat`) so the structured QML host can match the block back to
  `ContentsBodyResourceRenderer`'s resolved asset entry and paint the real package payload instead of the
  `.wsresource` directory path itself.
- `hasRenderedBlocks` now reflects any explicit document block, including semantic text-tag blocks, standalone
  `</break>`, and `<resource ... />`.
- `hasNonResourceRenderedBlocks` now likewise includes semantic text-tag blocks in addition to agenda/callout/break,
  letting hosts distinguish resource-only block sequences from notes that still own explicit editable prose blocks.
- Source refresh no longer runs the full structured verification pass once per backend signal. The renderer now parses the
  needed backends first, pulls their cached verification snapshots, and computes the combined structured verification
  once per source refresh.
- That combined verification now includes the file-layer synthetic XML/body validation for supported semantic tags, so
  renderer consumers can see malformed `paragraph`/`title`/`subTitle`/`event*`/`resource` source through the same
  `structuredParseVerification` payload instead of only agenda/callout/break-specific lint.
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
The newest regression also showed that resource-bearing notes could mount the structured flow yet fail to paint their
ordinary prose when semantic body tags were left inside raw gap text instead of becoming parser-owned text blocks.
