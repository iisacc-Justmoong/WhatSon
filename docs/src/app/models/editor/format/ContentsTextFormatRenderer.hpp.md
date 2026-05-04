# `src/app/models/editor/format/ContentsTextFormatRenderer.hpp`

## Responsibility

Declares the editor-side document renderer that converts `.wsnbody` source into the editor's document HTML projection
and, only when explicitly requested, also materializes markdown-aware preview HTML.

Highlight styling is delegated to the dedicated `ContentsTextHighlightRenderer` module so palette logic does not stay
embedded in the generic inline-tag parser.

## Public Contract
- `sourceText`
  Raw editor text payload (plain text plus inline tags such as `<bold>...</bold>`).
- `editorSurfaceHtml`
  Source-editing RichText HTML consumed by the live editor surface.
  Markdown syntax now stays literal here and is treated as ordinary `.wsnbody` source text instead of a separate
  display grammar.
- `htmlTokens`
  Parser-derived HTML token payloads published by the new block render pipeline.
- `normalizedHtmlBlocks`
  Stable HTML block payloads derived from `htmlTokens`.
  QML can now bind to one explicit render decision instead of rediscovering semantic/headline rendering from raw source
  strings.
- `htmlOverlayVisible`
  Tells the live editor surface whether the normalized HTML block output should visually replace the plain-text paint
  path for the current source snapshot.
- `renderedHtml`
  Optional markdown-aware preview HTML consumed by preview-only QML surfaces.
- `previewEnabled`
  Gates whether the expensive markdown-aware preview HTML should be recomputed at all.
- `ContentsHtmlBlockRenderPipeline`
  The live editor HTML path now goes through this dedicated pipeline:
  RAW source -> parser -> HTML tokens -> normalized HTML blocks -> final editor HTML.
  The renderer still keeps a carry-aware legacy whole-document fallback for multi-block inline-style spans until that
  style-state transfer is fully migrated into the block pipeline.
- `renderRichText(sourceText)`
  Stateless helper to render preview HTML without mutating bridge ownership state.
  This still includes markdown-style block rendering for:
    - ordered list prefixes such as `1. `
    - unordered list prefixes such as `- ` / `* ` / `+ `
    - headings (`#` ... `######`)
    - blockquotes (`> `)
    - fenced code blocks (`` ``` ``)
    - inline code and link-shaped literals
- `normalizeInlineStyleAliasesForEditor(sourceText)`
  Builds the cheaper source-editing HTML surface.
  Markdown glyphs remain literal source text here; only proprietary `.wsnbody` inline tags are promoted into styled
  spans for editing.
- `plainTextFromEditorSurfaceHtml(richTextHtml)`
  Normalizes one Qt RichText editor-surface HTML snapshot back into visible plain text.
  This is a read-side helper only; QML uses it to recover the user-visible text from Qt's serialized
  `<!DOCTYPE HTML ... qrichtext ...>` payload without ever treating that HTML document as writable `.wsnbody` source.
- `requestRenderRefresh()`
  Slot entrypoint for explicit refresh requests from QML when immediate recompute is needed.

## Supported Inline Style Tags
- Bold aliases: `bold`, `b`, `strong` -> `<strong style="font-weight:900;">`
- Italic aliases: `italic`, `i`, `em` -> `<span style="font-style:italic;">`
- Underline aliases: `underline`, `u` -> `<span style="text-decoration: underline;">`
- Strike aliases: `strikethrough`, `strike`, `s`, `del` -> `<span style="text-decoration: line-through;">`
- Highlight aliases: `highlight`, `mark` -> Apple Notes-inspired styled `<span ...>`

## Supported Structural Inline Tags
- Line break aliases: `br` -> `<br/>`
- Divider aliases: canonical `</break>` plus legacy `<hr ...>` -> rendered `<hr/>` divider

Markdown emphasis markers such as `**bold**`, `*italic*`, `~~strike~~`, or `==highlight==` are intentionally **not**
the formatting source of truth in this editor. Those styles remain bound to the proprietary `.wsnbody` inline tags and
the existing tag-insertion pipeline. Selection-driven RAW formatting now lives in
`src/app/models/editor/tags/ContentsEditorTagInsertionController.*`, and plain-text source replacement now lives in
`ContentsPlainTextSourceMutator`, not in this renderer bridge.

Markdown presentation roles are now emitted through `WhatSonNoteMarkdownStyleObject` only for explicit preview HTML.
The live editor surface no longer treats markdown as a second formatting authority.
