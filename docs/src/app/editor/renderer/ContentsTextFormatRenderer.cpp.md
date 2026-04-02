# `src/app/editor/renderer/ContentsTextFormatRenderer.cpp`

## Responsibility
Implements inline-format rendering from note-editor text to RichText HTML.

## Key Behavior
- Normalizes CRLF/CR to LF before parsing.
- Escapes plain text segments to keep HTML output safe.
- Converts supported inline style tags to explicit RichText/CSS spans:
  - `bold`/`b`/`strong` -> `<span style="font-weight:800;">`
  - `italic`/`i`/`em` -> `<span style="font-style:italic;">`
  - `underline`/`u` -> `<span style="text-decoration: underline;">`
  - `strikethrough`/`strike`/`s`/`del` -> `<span style="text-decoration: line-through;">`
- Routes `highlight` / `mark` tags through `ContentsTextHighlightRenderer` and renders an Apple Notes-inspired
  highlight span (`#8A4B00` background, `#FFD9A3` foreground, semibold text).
- Converts `<br>` style tags and newline characters to `<br/>`.
- Drops `<resource ...>` tags from text rendering so resource metadata is handled by dedicated resource renderers.
- Escapes unsupported tags as literal text instead of executing arbitrary markup.
- Recognizes supported `<span style=...>` runs and folds them back into the same canonical style stack, so stored
  `.wsnbody` aliases and editor-generated HTML spans render through one path.
- Auto-closes unmatched open style tags at end-of-input to keep emitted HTML structurally valid.
- Emits `renderedHtmlChanged()` only when the derived HTML payload actually changes.
- Exposes `normalizeInlineStyleAliasesForEditor(...)` for editor-surface normalization:
  - rewrites inline aliases into editable RichText spans (`bold` -> `<span style="font-weight:800;">`, etc.)
  - preserves non-style tags such as `<resource ...>` unchanged
  - preserves escaped safe text such as `&lt;bold&gt;...&lt;/bold&gt;` unchanged
- Exposes `normalizeEditorSurfaceTextToSource(...)` so the RichText editor surface can be canonicalized back into
  inline `.wsnbody` tags after direct text edits.
- Preview HTML generation and editable-surface normalization now reuse the same explicit span-style openings, so
  read-side rendering and editor rendering no longer diverge on weight/decoration styling.
