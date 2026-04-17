# `src/app/editor/renderer/ContentsHtmlBlockRenderPipeline.hpp`

## Responsibility

Declares the RAW-to-HTML block pipeline used by editor-facing renderers.

This pipeline sits after `ContentsWsnBodyBlockParser` and before the final RichText consumer:

1. parse canonical RAW `.wsnbody` source into ordered document blocks
2. convert those parser-owned blocks into editor HTML tokens
3. resolve one render delegate / HTML fragment per token
4. normalize the token stream into stable HTML blocks plus one joined editor document HTML payload

## Public Contract
- `RenderResult.documentHtml`
  Final editor HTML document assembled from normalized HTML blocks.
- `RenderResult.htmlTokens`
  Block-granular HTML token stream derived from the parser result.
- `RenderResult.normalizedHtmlBlocks`
  Stable HTML block payloads with block/token indices for downstream renderers.
- `RenderResult.htmlOverlayVisible`
  Tells QML whether the current source should paint the styled HTML overlay instead of showing only the plain input
  text.
- `RenderResult.requiresLegacyDocumentComposition`
  Signals that the caller should keep the older carry-aware whole-document HTML composer for this source snapshot.
  The initial pipeline revision intentionally keeps that escape hatch for multi-block inline-style spans.

## Notes
- Textual blocks are still rendered through `WhatSonNoteBodyPersistence` so inline-tag semantics stay aligned with the
  canonical note-body serializer/parser.
- Semantic text block types such as `title`, `subTitle`, and `eventTitle` now surface their heading styling through
  this normalized HTML-block layer instead of relying on ad hoc QML-only heuristics.
