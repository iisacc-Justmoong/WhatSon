# `src/app/qml/view/content/editor/ContentsLogicalLineLayoutSupport.js`

## Responsibility

Provides one shared logical-line geometry helper for structured text-family block delegates that need gutter/minimap
line coordinates from live `positionToRectangle(...)` data.

## Public Helpers

- `normalizedPlainTextLines(plainTextValue)`
  Normalizes the visible plain-text editor snapshot into the logical-line list consumed by the geometry builder.
- `buildEntries(plainTextValue, blockHeight, editorItem, mapTarget, fallbackLineHeight)`
  Builds one `{contentY, contentHeight}` entry per logical line and maps every sampled `TextEdit` rectangle into the
  owning block's coordinate space before the structured flow consumes it.

## Notes

- The helper is intentionally shared by `ContentsDocumentTextBlock.qml` and `ContentsCalloutBlock.qml` so both blocks
  derive logical-line Y from the same coordinate contract.
- `positionToRectangle(...)` is sampled from the live inline editor, but the helper now always runs that geometry
  through `mapToItem(...)` when available.
  Logical-line caches therefore stay aligned even when the live editor's internal text item is no longer rooted at the
  block's local `y = 0`.
- The helper remains read-side only.
  It does not mutate RAW source and does not own persistence.

## Tests

- Automated regression coverage now executes this helper through `QJSEngine` in
  `test/cpp/whatson_cpp_regression_tests.cpp`.
- Regression checklist:
  - mapped editor offsets must shift every logical line entry by the same parent-coordinate delta
  - line heights must still come from the live editor rectangles instead of collapsing to one fixed fallback height
  - fallback distribution must still produce one entry for empty/plain one-line blocks when no editor geometry exists
