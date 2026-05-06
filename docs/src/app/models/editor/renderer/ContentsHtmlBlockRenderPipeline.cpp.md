# `src/app/models/editor/renderer/ContentsHtmlBlockRenderPipeline.cpp`

## Responsibility
Builds editor HTML tokens and normalized HTML blocks from parser-owned RAW document blocks.

## Key Behavior
- Runs `ContentsWsnBodyBlockParser` first and treats that ordered block list as the only source of block-level render
  truth.
- If the parser/linter suggests a deterministic canonical RAW projection that differs from the incoming snapshot,
  reparses that corrected source before assembling final HTML tokens and blocks.
  This keeps legacy divider aliases such as `<hr>` and other safe structured-tag repairs aligned with the same final
  block projection that the editor surface publishes.
- Converts each parsed block into one normalized HTML token carrying:
  - block index / token index
  - block type
  - resolved render delegate type (`text`, `agenda`, `callout`, `resource`, `break`)
  - source span
  - normalized HTML fragment
  - overlay-visibility flag
  - the parser-owned block metadata, including resource identifiers and paths for `resource` blocks
- Builds a renderer-local XML projection for each HTML token, validates it with `iiXml::Parser::TagParser`, converts it
  through `iiHtmlBlock::iiXmlToHTML`, and divides it with `iiHtmlBlock::DivideBlock`.
- Converts the resulting iiHtmlBlock display-block objects into `normalizedHtmlBlocks` without assuming one token maps
  to exactly one block. If one token yields several display blocks, the renderer publishes several normalized blocks
  with the same `htmlTokenStartIndex`.
- Each normalized HTML block now includes iiHtmlBlock metadata:
  - `htmlBlockObjectSource=iiHtmlBlock`
  - `htmlBlockTagName`
  - raw/value ranges
  - `htmlBlockIsDisplayBlock`
  - display override fields
- Delegates textual fragment rendering to
  `WhatSon::ContentsTextFormatRendererInternal::renderInlineTaggedTextFragmentToHtml(..., SourceEditing)` so stored
  proprietary inline tags such as `<bold>` and `<italic>` become editor RichText spans instead of being reprojected
  through the persistence layer's plain-text `bodyPlainLinesFromDocument(...)` path.
- Textual fragments are split back into paragraph-flow HTML after that rich-text projection. Newlines produced by
  ordinary Enter therefore materialize as real `<p ...>` slots, and empty lines become `&nbsp;` paragraphs instead of
  remaining as zero-height trailing `<br/>` breaks.
- Keeps resource placeholders in the stable
  `<!--whatson-resource-block:N--> ... <!--/whatson-resource-block:N-->` form expected by the inline resource
  replacement controller, while preserving the source `resourcePath`, `resourceId`, `resourceType`, and
  `resourceFormat` on the corresponding token and normalized iiHtmlBlock payload.
- Still exposes `requiresLegacyDocumentComposition` for the transitional case where one source snapshot contains
  inline-style markup that spans several parsed text blocks.
  That prevents the new block-normalized path from dropping carried style state before the whole-document carry model
  is fully migrated.

## Regression Checks
- Parsed text blocks that only differ by semantic block type (`paragraph`, `title`, `subTitle`, `eventTitle`) must
  still resolve to `renderDelegateType=text` while preserving their distinct semantic styling in the HTML fragment.
- Explicit paragraph content that contains Enter-authored newlines must render as separate editor paragraph slots, with
  empty lines backed by `&nbsp;`, so the visible document flow is pushed down like a normal text editor.
- Stored inline style tags such as `<bold>Al<italic>pha</italic></bold><italic> Beta</italic>` must render as RichText
  style HTML in `documentHtml`, `htmlTokens`, and `normalizedHtmlBlocks`; escaped literal `<bold>` / `<italic>` text is
  a renderer regression.
- A `resource` block must normalize to one stable HTML block with the same placeholder marker indices expected by
  `ContentsInlineResourcePresentationController`.
- A legacy divider alias such as `<hr>After` must first canonicalize to `</break>After`, then normalize into separate
  `break` and trailing `text` HTML blocks so the editor surface does not lose the following prose.
- A multi-block note whose inline style tag opens in one text block and closes in a later text block must force the
  legacy document-composition fallback instead of silently dropping the carried style in the final editor HTML.
