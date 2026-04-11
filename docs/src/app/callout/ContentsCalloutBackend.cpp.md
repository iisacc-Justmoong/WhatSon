# `src/app/callout/ContentsCalloutBackend.cpp`

## Responsibility
Implements callout source parsing and insertion helpers shared by desktop/mobile editor surfaces.

## Key Behaviors
- Parses `<callout ...>...</callout>` blocks into QML-friendly entries (`text`).
- Canonicalizes callout display text for rendering:
  - normalizes line endings
  - preserves source line breaks (`\n`) and maps `<br>` aliases to line breaks
  - decodes safe entities (`&lt;`, `&gt;`, `&amp;`, `&quot;`, `&#39;`, `&nbsp;`)
  - strips nested source tags from rendered callout text projection
- Builds canonical callout insertion payloads:
  - `<callout>...</callout>`
  - cursor placement offset inside callout body text.

## Architectural Notes
- Callout parsing/mutation regex and entity handling are intentionally localized here so QML controllers only manage
  event flow and cursor orchestration.
- The module does not perform persistence directly; it only returns transformed source text or insertion metadata.

## Regression Checklist
- `<callout>Message</callout>` must parse as one callout entry.
- `buildCalloutInsertionPayload("abc")` must return canonical source `<callout>abc</callout>` with cursor inside
  the callout body.
- `buildCalloutInsertionPayload(...)` must escape raw `<`, `>`, `&`, and quotes in inserted body text.
- `<callout>Line 1<br/>Line 2</callout>` must parse into display text with preserved line breaks.
