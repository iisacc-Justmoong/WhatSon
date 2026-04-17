# `src/app/editor/renderer/ContentsHtmlBlockRenderPipeline.cpp`

## Responsibility
Builds editor HTML tokens and normalized HTML blocks from parser-owned RAW document blocks.

## Key Behavior
- Runs `ContentsWsnBodyBlockParser` first and treats that ordered block list as the only source of block-level render
  truth.
- Converts each parsed block into one normalized HTML token carrying:
  - block index / token index
  - block type
  - resolved render delegate type (`text`, `agenda`, `callout`, `resource`, `break`)
  - source span
  - normalized HTML fragment
  - overlay-visibility flag
- Converts those tokens into `normalizedHtmlBlocks` without letting QML rediscover block-flow ownership or semantic
  text styling from raw strings.
- Uses `WhatSonNoteBodyPersistence::serializeBodyDocument(...)` plus
  `WhatSonNoteBodyPersistence::htmlProjectionFromBodyDocument(...)` for textual fragments so inline-tag rendering stays
  aligned with the canonical body serializer.
- Keeps resource placeholders in the stable
  `<!--whatson-resource-block:N--> ... <!--/whatson-resource-block:N-->` form expected by the inline resource
  replacement controller.
- Still exposes `requiresLegacyDocumentComposition` for the transitional case where one source snapshot contains
  inline-style markup that spans several parsed text blocks.
  That prevents the new block-normalized path from dropping carried style state before the whole-document carry model
  is fully migrated.

## Regression Checks
- Parsed text blocks that only differ by semantic block type (`paragraph`, `title`, `subTitle`, `eventTitle`) must
  still resolve to `renderDelegateType=text` while preserving their distinct semantic styling in the HTML fragment.
- A `resource` block must normalize to one stable HTML block with the same placeholder marker indices expected by
  `ContentsInlineResourcePresentationController.qml`.
- A multi-block note whose inline style tag opens in one text block and closes in a later text block must force the
  legacy document-composition fallback instead of silently dropping the carried style in the final editor HTML.
