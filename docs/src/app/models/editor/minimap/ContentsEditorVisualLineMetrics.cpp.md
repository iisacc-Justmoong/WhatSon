# `src/app/models/editor/minimap/ContentsEditorVisualLineMetrics.cpp`

## Role

Implements visual-line snapshot normalization for minimap rows.

## Calculation Rules

- `visualLineCount` is the greater of the measured count and the number of measured width-ratio entries.
- `visualLineWidthRatios` clamps supplied measured ratios into `[0, 1]`.
- Rows without measured ratios default to full-width minimap rows.
- No TextEdit/QQuickItem methods are called from this object.

## Verification

Covered by `contentsEditorVisualLineMetrics_expandsTallVisualBlocks` and the QML inline editor minimap runtime test.
