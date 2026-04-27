# `src/app/models/editor/renderer/ContentsStructuredBlockRenderer.cpp`

## Responsibility
Builds canonical structured render data from `.wsnbody` source text.

## Current Contract
- Still exposes `renderedAgendas` and `renderedCallouts` for legacy/fallback consumers.
- Now also exposes `renderedDocumentBlocks`, a mixed document-flow model ordered by source position.
- `renderedDocumentBlocks` interleaves:
  - implicit prose paragraphs split from plain-text line gaps: `type=paragraph`
  - agenda blocks: `type=agenda`
  - callout blocks: `type=callout`
  - resource blocks: `type=resource`
  - divider tags: `type=break`
- The renderer now delegates that top-level body parsing to `ContentsWsnBodyBlockParser` and simply republishes the
  parser snapshot to QML.
  In other words, `paragraph`, `title`, `resource`, `agenda`, `callout`, and `break` now enter the editor-facing
  document flow through one parser-owned block-span contract instead of a renderer-local merge of several tag-specific
  read paths.
- Canonical semantic body elements such as `paragraph`, `p`, `title`, `subTitle`, `eventTitle`, and
  `eventDescription` now materialize as explicit block spans whose `type` is the canonical tag name itself
  (for example `type=paragraph`, `type=title`, `type=subtitle`, `type=eventtitle`).
  Resource-bearing notes therefore keep their authored prose blocks visible in the structured document flow instead of
  relying on one monolithic pre-resource text fragment.
- Parser-owned plain prose that is not wrapped in a semantic text tag is now also published paragraph-by-paragraph.
  Newline-delimited blank prose slots below an inline resource therefore survive as their own text-editable document
  blocks instead of being merged into one multiline fallback block.
- Resource-only source that ends with a newline now also publishes the trailing blank line as a text-editable
  `text-group`. That keeps the gutter and caret target for the empty paragraph immediately after an image/resource
  block.
- Empty source lines between two resource blocks likewise publish as a text-editable `text-group`; the renderer must not
  collapse adjacent atomic blocks together when RAW contains an authored empty paragraph between them.
- Each explicit block, including semantic text-tag blocks, now carries parser-owned source geometry so QML can rewrite
  RAW in place.
- The renderer now also preserves the parser's generic block-trait payload (`plainText`, `textEditable`,
  `atomicBlock`, `gutterCollapsed`, `logicalLineCountHint`, `minimapVisualKind`,
  `minimapRepresentativeCharCount`) instead of forcing QML to rediscover those traits from block type names.
- The renderer-side placeholder payload now normalizes `logicalLineCountHint` to the same `int`-sized contract as the
  parser path, preventing host/compiler-dependent integer width mismatches during placeholder publication.
- Explicit blocks now also carry one normalized `tagName` alongside `type`, so downstream QML sees the same
  document-block identity contract regardless of which supported tag family produced the span.
- Semantic text-tag blocks additionally split wrapper span from editable content span:
  - `blockSourceStart` / `blockSourceEnd` keep the outer semantic tag
  - `sourceStart` / `sourceEnd` / `sourceText` now point at only the editable inner content
  This prevents `ContentsDocumentTextBlock.qml` from treating `<paragraph>` or `<title>` wrappers as visible text or
  replacing the whole wrapper when the user edits only the inner prose.
- Resource blocks also carry a stable `resourceIndex` plus the canonical tag attributes (`resourceId`,
  `resourcePath`, `resourceType`, `resourceFormat`) so the structured QML host can match the block back to
  `ContentsBodyResourceRenderer`'s resolved asset entry and paint the real package payload instead of the
  `.wsresource` directory path itself.
- That same resource-block payload is now also the only regular-note input contract for
  `ContentsBodyResourceRenderer`.
  The resource renderer no longer re-reads `.wsnbody` or runs an independent `<resource ...>` regex scan, so parser
  order/source spans and resolved resource payload now come from one shared block stream.
- `hasRenderedBlocks` now reflects any explicit document block, including semantic text-tag blocks, standalone
  `</break>`, and `<resource ... />`.
- `hasNonResourceRenderedBlocks` now likewise includes semantic text-tag blocks in addition to agenda/callout/break,
  letting hosts distinguish resource-only block sequences from notes that still own explicit editable prose blocks.
- Source refresh no longer reparses separate read-side backends and then merges them. The renderer now asks the parser
  for one snapshot and forwards that snapshot's verification payloads directly.
- That combined verification now includes the file-layer synthetic XML/body validation for supported semantic tags, so
  renderer consumers can see malformed `paragraph`/`title`/`subTitle`/`event*`/`resource` source through the same
  `structuredParseVerification` payload instead of only agenda/callout/break-specific lint.
- Notes that do not contain any proprietary structured tags now still fall back entirely through the parser, but the
  fallback is no longer "one giant text block"; it is one ordered paragraph stream derived from RAW line breaks.
- When the host enables `backgroundRefreshEnabled`, notes that may contain agenda/callout/resource/break blocks now
  publish a cheap single-text-block placeholder immediately and compute the expensive structured render snapshot on a
  worker thread.
- The renderer still keeps tiny local fast-path helpers for that background gate and placeholder publish path:
  case-insensitive tag probes (`mayContainAgendaBlock`, `mayContainCalloutBlock`, `mayContainResourceBlock`,
  `mayContainBreakBlock`) plus a minimal `documentBlockPayload(...)` builder for the temporary plain-text placeholder.
  That placeholder builder now mirrors the same generic block-trait shape as the parser result, so empty/initial
  structured hosts do not fall back to a different payload contract.
  Those helpers are intentionally local renderer utilities; they do not reintroduce the old multi-backend parse path.
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
The same transition also exposed that semantic text-tag blocks needed an explicit parser contract for inner editable
content, otherwise the structured paragraph editor would receive outer wrapper tags as visible text and could strip
those wrappers on the next RAW rewrite.
