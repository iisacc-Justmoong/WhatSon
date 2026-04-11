# `src/app/qml/view/content/editor/ContentsStructuredCursorSupport.js`

## Responsibility
Provides shared plain-text cursor/source-offset conversions for structured editor blocks.

## Key Behavior
- Normalizes line endings and whitespace consistently with the agenda/callout backends.
- Escapes plain text into canonical RAW source content for `<task>` and `<callout>` bodies.
- Maps a visible plain-text caret position to the corresponding RAW source offset, including entity-expanded characters.
- Maps a RAW source offset back to a visible plain-text caret position after reparsing.
