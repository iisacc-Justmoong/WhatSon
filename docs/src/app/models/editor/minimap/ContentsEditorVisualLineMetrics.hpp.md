# `src/app/models/editor/minimap/ContentsEditorVisualLineMetrics.hpp`

## Role

Declares the QObject that measures the live editor surface's visible line rows for minimap rendering.

## Contract

- QML supplies the currently visible TextEdit object and primitive geometry inputs.
- The class publishes `visualLineCount` and `visualLineWidthRatios`.
- Wrapped text contributes one minimap row per visible row.
- Tall rendered blocks contribute enough rows to cover their rendered height.
- QML must not duplicate line-count or per-row width calculations.

## Boundary

This object does not render minimap rows and does not mutate note source. It only reads visible editor geometry and
returns presentation metrics to the QML view.
