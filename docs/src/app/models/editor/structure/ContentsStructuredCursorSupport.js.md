# `src/app/models/editor/structure/ContentsStructuredCursorSupport.js`

## Responsibility
Provides shared plain-text cursor/source-offset conversions for structured editor blocks.

## Key Behavior
- Normalizes line endings and whitespace consistently with the agenda/callout backends.
- Escapes plain text into canonical RAW source content for `<task>` and `<callout>` bodies.
- Maps a visible plain-text caret position to the corresponding RAW source offset, including entity-expanded characters.
- Visible boundaries that land immediately after a closing inline-style run now advance past consecutive zero-width
  closing proprietary tags such as `</bold>`, `</italic>`, `</underline>`, `</strikethrough>`, and `</highlight>`.
- The structured text-block editor therefore treats the end of a styled run as a position after the closing tag, not
  as a splice point inside RAW markup.
- Maps a RAW source offset back to a visible plain-text caret position after reparsing.
