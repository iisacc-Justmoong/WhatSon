# `src/app/editor/renderer/ContentsTextFormatRenderer.cpp`

## Responsibility
Implements inline-format rendering from note-editor text to RichText HTML.

## Key Behavior
- Normalizes CRLF/CR to LF before parsing.
- Escapes plain text segments to keep HTML output safe.
- Recognizes markdown-style block prefixes without rewriting the stored source text:
    - unordered list markers (`- ` / `* ` / `+ `) render as bullet-list lines
    - leading literal bullet markers (`• `) also render through the same unordered-list role so pasted or legacy source
      lines do not fall back to plain black text in print/editor RichText views
    - ordered list markers (`1. ` / `2. ` / `3) `) render as numbered-list lines
    - heading markers (`#` ... `######`) render as larger title lines while keeping the marker text visible
    - blockquotes (`> `) render as muted italic quote lines
    - fenced code blocks keep the literal `` ``` `` lines but render the fenced region with monospace/code styling
- The RichText spans for those markdown roles are emitted through `WhatSonNoteMarkdownStyleObject`, not ad-hoc
  free-form CSS strings.
- Converts supported inline style tags to explicit RichText/CSS spans:
  - `bold`/`b`/`strong` -> `<strong style="font-weight:900;">`
  - `italic`/`i`/`em` -> `<span style="font-style:italic;">`
  - `underline`/`u` -> `<span style="text-decoration: underline;">`
  - `strikethrough`/`strike`/`s`/`del` -> `<span style="text-decoration: line-through;">`
- Routes `highlight` / `mark` tags through `ContentsTextHighlightRenderer` and renders an Apple Notes-inspired
  highlight span (`#8A4B00` background, `#D6AE58` foreground, semibold text).
- Styles inline markdown-shaped literals that do not conflict with proprietary formatting tags:
    - inline code spans (`` `code` ``)
    - link-shaped literals (`[label](url)`)
- Proprietary formatting remains authoritative for bold/italic/underline/strikethrough/highlight; markdown emphasis
  tokens are intentionally not promoted into those styles.
- Converts `<br>` style tags and newline characters to `<br/>`.
- Drops `<resource ...>` tags from text rendering so resource metadata is handled by dedicated resource renderers.
- Escapes unsupported tags as literal text instead of executing arbitrary markup.
- Recognizes supported `<span style=...>` runs and folds them back into the same canonical style stack, so stored
  `.wsnbody` aliases and editor-generated HTML spans render through one path.
- Auto-closes unmatched open style tags at end-of-input to keep emitted HTML structurally valid.
- Emits `renderedHtmlChanged()` only when the derived HTML payload actually changes.
- Decodes one entity layer from safe literal text fragments (`&amp;`, `&lt;`, `&gt;`, `&quot;`, `&#39;`, `&nbsp;`)
  before emitting RichText HTML, so RAW-safe source escapes render as their real glyphs in the editor while the
  canonical source can still keep escaped tokens.
- Exposes `normalizeInlineStyleAliasesForEditor(...)` for editor-surface normalization:
  - rewrites inline aliases into editable RichText tags (`bold` -> `<strong style="font-weight:900;">`, etc.)
  - preserves non-style tags such as `<resource ...>` unchanged
  - promotes canonical source LF characters to explicit `<br/>` tags before binding into `TextEdit.RichText`
  - renders escaped safe text such as `&lt;bold&gt;...&lt;/bold&gt;` as visible `<bold>...</bold>` glyphs without
    promoting it into authoritative inline-style tags
- Exposes `normalizeEditorSurfaceTextToSource(...)` so formatting-driven `QTextDocument` mutations can still be
  canonicalized back into inline `.wsnbody` tags when the whole RichText surface genuinely changed.
- That whole-surface normalization also converts rendered unordered-list bullet glyphs (`• `) back into source markdown
  markers (`- `), so selection-format round-trips do not silently replace markdown list syntax with literal bullets.
  That canonical `-` marker is now an explicit shared policy.
- Exposes `applyPlainTextReplacementToSource(...)` so ordinary typing can mutate only the affected raw source span:
  - accepts canonical source text plus source start/end offsets
  - normalizes plain-text line endings
  - clamps source offsets against an `int`-safe `QString` length before replacement
  - escapes inserted literal text before stitching it back into `.wsnbody`
  - avoids whole-document RichText export for normal typing/backspace/delete/paste
- Exposes `applyInlineStyleToSelectionSource(...)` so inline formatting no longer depends on QML string splicing:
  - loads the current editor RichText surface into `QTextDocument`
  - resolves the selected range with `QTextCursor`
  - accepts `plain` / `clear` / `none` as explicit remove-formatting commands for context-menu usage
  - if the entire selection already has the requested style, replaces that selection with an explicit plain-text
    `QTextCharFormat`, so the matching inline source tags are removed from canonical `.wsnbody` output instead of
    surviving as hidden markup
  - otherwise merges `QTextCharFormat` for `bold` / `italic` / `underline` / `strikethrough` / `highlight`
  - serializes the updated document back into canonical `.wsnbody`
- Exposes `applyInlineStyleToLogicalSelectionSource(...)` for shortcut/context-menu formatting that must ignore markdown
  presentation roles:
    - builds a markdown-neutral inline-style editing surface from canonical source text
    - resolves the selected range by logical editor offsets
    - toggles only when the proprietary `.wsnbody` inline style is already present
    - serializes the edited document back into canonical `.wsnbody`
- Preview HTML generation and editable-surface normalization now reuse the same strong/span-style openings, so
  read-side rendering and editor rendering no longer diverge on weight/decoration styling.

## Regression Checks

- Typing `- item` in the note body should re-render as a bullet-list line without changing the stored source marker.
- A stored source line that already begins with `• ` must render with the same unordered-list marker styling as `- `
  instead of falling back to plain black text in print/editor RichText views.
- Typing `1. item` should re-render as a numbered-list line.
- Typing `# title` should re-render as a heading-like line while preserving literal `# ` in the source text.
- Typing `` ``` `` fenced blocks should keep the fence markers visible and render the fenced body as code-styled text.
- Typing `[label](https://example.com)` should keep the literal token in source while rendering it as a link-styled
  span.
- Safe RAW entity tokens such as `&lt;`, `&gt;`, and `&amp;` must render as visible `<`, `>`, and `&` glyphs in the
  editor surface without being promoted into executable markup.
- Escaped safe text such as `&lt;bold&gt;demo&lt;/bold&gt;` must display the literal angle-bracket text in the editor
  instead of the raw entity strings, while proprietary formatting still ignores it as source text.
- Applying proprietary inline formatting to text inside a markdown bullet line must keep the stored source line prefix
  as
  `- ` instead of persisting the rendered `• ` glyph.
- Applying `Bold`/`Italic`/`Underline`/`Highlight` shortcuts to heading, blockquote, link-literal, or code-literal text
  must still add/remove proprietary `.wsnbody` tags instead of misreading markdown presentation styling as an
  already-applied shortcut format.
