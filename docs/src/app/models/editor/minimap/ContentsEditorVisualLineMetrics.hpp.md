# `src/app/models/editor/minimap/ContentsEditorVisualLineMetrics.hpp`

## Role

Declares the QObject that normalizes measured visible-line snapshots for minimap rendering.

## Contract

- QML supplies measured visual-line count and measured row-width ratios produced by the geometry adapter.
- The class publishes `visualLineCount` and `visualLineWidthRatios`.
- Wrapped text contributes one minimap row per visible row.
- Tall rendered blocks contribute enough rows to cover their rendered height.
- This object must not hold TextEdit, cursor, selection, resource overlay, or QQuickItem references.

## Boundary

This object does not render minimap rows and does not mutate note source. It only consumes measured snapshot values and
returns presentation metrics to the QML view.
