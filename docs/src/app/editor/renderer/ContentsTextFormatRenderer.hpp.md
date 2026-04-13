# `src/app/editor/renderer/ContentsTextFormatRenderer.hpp`

## Responsibility

Declares the editor-side renderer bridge that converts `.wsnbody` inline style tags into the editor's source-editing
RichText surface and, only when explicitly requested, also materializes markdown-aware preview HTML.

Highlight styling is delegated to the dedicated `ContentsTextHighlightRenderer` module so palette logic does not stay
embedded in the generic inline-tag parser.

## Public Contract
- `sourceText`
  Raw editor text payload (plain text plus inline tags such as `<bold>...</bold>`).
- `editorSurfaceHtml`
  Source-editing RichText HTML consumed by the live editor surface.
  Markdown syntax now stays literal here and is treated as ordinary `.wsnbody` source text instead of a separate
  display grammar.
- `renderedHtml`
  Optional markdown-aware preview HTML consumed by preview-only QML surfaces.
- `previewEnabled`
  Gates whether the expensive markdown-aware preview HTML should be recomputed at all.
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
- `applyPlainTextReplacementToSource(sourceText, sourceStart, sourceEnd, replacementText)`
  Replaces one source span with escaped plain text directly in canonical `.wsnbody`.
  Source bounds are clamped against an `int`-safe `QString` length before stitching.
  This is the ordinary typing path and intentionally does not reserialize the entire RichText surface.
- `applyInlineStyleToLogicalSelectionSource(sourceText, selectionStart, selectionEnd, styleTag)`
  Applies inline formatting from canonical source text plus logical editor offsets by rebuilding proprietary inline
  source tags directly from RAW-source style coverage.
  Logical selection offsets are now resolved against the stored source text itself, and the opening/closing proprietary
  tags become the authoritative formatting boundaries instead of any transient `QTextDocument` fragment split.
  Surface HTML is intentionally outside this write path so inline-format commands cannot promote the rendered editor
  projection into a second source of truth.
- `requestRenderRefresh()`
  Slot entrypoint for explicit refresh requests from QML when immediate recompute is needed.

## Supported Inline Style Tags
- Bold aliases: `bold`, `b`, `strong` -> `<strong style="font-weight:900;">`
- Italic aliases: `italic`, `i`, `em` -> `<span style="font-style:italic;">`
- Underline aliases: `underline`, `u` -> `<span style="text-decoration: underline;">`
- Strike aliases: `strikethrough`, `strike`, `s`, `del` -> `<span style="text-decoration: line-through;">`
- Highlight aliases: `highlight`, `mark` -> Apple Notes-inspired styled `<span ...>`
- Clear aliases: `plain`, `clear`, `none` -> remove inline formatting from the selected range

## Supported Structural Inline Tags
- Line break aliases: `br` -> `<br/>`
- Divider aliases: canonical `</break>` plus legacy `<hr ...>` -> rendered `<hr/>` divider

Markdown emphasis markers such as `**bold**`, `*italic*`, `~~strike~~`, or `==highlight==` are intentionally **not**
the formatting source of truth in this editor. Those styles remain bound to the proprietary `.wsnbody` inline tags and
the existing shortcut/context-menu pipeline.

Markdown presentation roles are now emitted through `WhatSonNoteMarkdownStyleObject` only for explicit preview HTML.
The live editor surface no longer treats markdown as a second formatting authority.
