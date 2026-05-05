# `src/app/models/editor/minimap/ContentsEditorVisualLineMetrics.cpp`

## Role

Implements visible editor line measurement for minimap rows.

## Calculation Rules

- `visualLineCount` is the greater of the live TextEdit line count and `contentHeight / lineHeight`.
- `visualLineWidthRatios` probes each visible text row with TextEdit `positionAt(...)` and
  `positionToRectangle(...)`, then normalizes measured row width against the editor column width.
- Height-derived rows that cannot be probed from real text geometry default to full-width minimap rows.
- All numeric inputs are clamped before use so transient QML geometry changes cannot collapse the minimap to zero
  rows.

## Verification

Covered by `contentsEditorVisualLineMetrics_expandsTallVisualBlocks` and the QML inline editor minimap runtime test.
