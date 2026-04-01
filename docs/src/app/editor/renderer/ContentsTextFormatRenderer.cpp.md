# `src/app/editor/renderer/ContentsTextFormatRenderer.cpp`

## Responsibility
Implements inline-format rendering from note-editor text to RichText HTML.

## Key Behavior
- Normalizes CRLF/CR to LF before parsing.
- Escapes plain text segments to keep HTML output safe.
- Converts supported inline style tags to canonical HTML tags:
  - `bold`/`b`/`strong` -> `strong`
  - `italic`/`i`/`em` -> `em`
  - `underline`/`u` -> `u`
  - `strikethrough`/`strike`/`s`/`del` -> `s`
- Converts `<br>` style tags and newline characters to `<br/>`.
- Drops `<resource ...>` tags from text rendering so resource metadata is handled by dedicated resource renderers.
- Escapes unsupported tags as literal text instead of executing arbitrary markup.
- Auto-closes unmatched open style tags at end-of-input to keep emitted HTML structurally valid.
- Emits `renderedHtmlChanged()` only when the derived HTML payload actually changes.
