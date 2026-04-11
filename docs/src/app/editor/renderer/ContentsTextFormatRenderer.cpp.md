# `src/app/editor/renderer/ContentsTextFormatRenderer.cpp`

## Responsibility
Implements inline-format rendering from note-editor text to RichText HTML.

## Key Behavior
- Normalizes CRLF/CR to LF before parsing.
- Escapes plain text segments to keep HTML output safe.
- The renderer now maintains two different HTML outputs:
  - `editorSurfaceHtml`: cheap source-editing HTML for the live editor
  - `renderedHtml`: optional markdown-aware preview HTML, recomputed only while preview is enabled
- The live editor surface now treats markdown as raw `.wsnbody` source:
  - list markers, headings, blockquotes, fenced code fences, inline code, and markdown links remain literal text in
    `editorSurfaceHtml`
  - only proprietary inline tags such as `<bold>` / `<italic>` / `<highlight>` are promoted into styled HTML spans
- The preview path still recognizes markdown-style block prefixes without rewriting the stored source text:
    - unordered list markers (`- ` / `* ` / `+ `) render as bullet-list lines
    - leading literal bullet markers (`• `) also render through the same unordered-list role so pasted or legacy source
      lines do not fall back to plain black text in markdown preview views
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
- Converts the canonical single divider tag `</break>` (and legacy `<hr ...>` aliases) into rendered divider HTML
  (`<hr/>`) on both the live editor surface and preview HTML.
- Drops `<resource ...>` tags from text rendering so resource metadata is handled by dedicated resource renderers.
- Drops proprietary `<agenda ...>` / `<task ...>` wrapper tags from inline text rendering so agenda/task UI can be
  rendered by dedicated view layers without leaking raw wrapper markup.
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
  - deliberately stops promoting markdown syntax into list/heading/blockquote/code/link preview spans on the live
    editor surface
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
    - no longer routes the selection through `QTextDocument` fragment formatting as the source of truth
    - reuses the same local tag-token and entity classifiers as the rest of the renderer pipeline, so RAW-source
      coverage scans and HTML/source parsing stay aligned
    - instead scans the RAW editor source, computes logical character coverage for each proprietary inline style, and
      rebuilds canonical `<bold>` / `<italic>` / `<underline>` / `<strikethrough>` / `<highlight>` tags directly from
      that coverage
    - the selected range is therefore compared against the actual stored source tags, not against a temporary RichText
      fragment split that may collapse multi-tag or multi-paragraph selections
    - `plain` clears every supported proprietary inline style across the selected logical range
    - reapplying the same style toggles that style off by zeroing its RAW-source coverage inside the range
    - applying a style across text that is split by other proprietary tags now expands one continuous source-tag span
      over the whole logical selection
    - markdown line prefixes are now style-protected during RAW coverage rebuild:
      - leading indentation (`[ \t]*`)
      - unordered list prefixes (`- ` / `+ ` / `* ` / `• `)
      - ordered list prefixes (`N. ` / `N) `)
      - heading prefixes (`#` .. `######` + whitespace)
      - blockquote/fence prefixes (`>`, `` ``` ``)
      so inline style tags are not inserted in front of structural markers
- Preview HTML generation and editable-surface normalization now reuse the same strong/span-style openings, so
  read-side rendering and editor rendering no longer diverge on weight/decoration styling.

## Regression Checks

- Typing `- item` in the note body must keep `- ` visible on the live editor surface; the source marker must not be
  replaced by a rendered bullet glyph in that editing path.
- A stored source line that already begins with `• ` must render with the same unordered-list marker styling as `- `
  instead of falling back to plain black text in markdown preview views.
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
- Applying `Strikethrough` (or any inline style) to a range that crosses indented/list lines must not delete or
  visually collapse the line indentation; structural line prefixes must remain outside proprietary inline tags.
- Applying proprietary inline formatting to a selection that spans multiple existing proprietary tags or multiple
  logical paragraphs must rebuild the target RAW source tags over the whole selected range, not only the first RichText
  fragment that happened to survive the context-menu click.
- Applying `Bold`/`Italic`/`Underline`/`Highlight` shortcuts to heading, blockquote, link-literal, or code-literal text
  must still add/remove proprietary `.wsnbody` tags instead of misreading markdown presentation styling as an
  already-applied shortcut format.
- A stored `</break>` token must render as a visible divider line (not literal text) and still consume one logical
  break slot in inline-style selection coverage rebuild.
- A stored `<agenda>/<task>` block must not render raw wrapper tags as literal text on the live editor surface.
- When preview is disabled, mutating `sourceText` must not recompute markdown-aware preview HTML.
