# `src/app/qml/view/contents/Minimap.qml`

## Role
`Minimap.qml` renders the Figma minimap rail for node `352:8626`.

## Contract
- Imports LVRS and uses LVRS color and metric tokens for row spacing and line height.
- Exposes `rowCount` and `lineColor` for root-frame binding.
- Emits `viewHookRequested(string reason)` through `requestViewHook(reason)`.

## UI
- The token-composed host width is assigned by `ContentsView.qml`.
- The minimap is a clipped column of repeated LVRS hairline rows.
- Row-count and width policy lives in `src/app/models/editor/minimap`; this file only renders the resolved count.
- The default row count is a token-only placeholder. Real design/runtime values are supplied by the host from
  `ContentsMinimapLayoutMetrics`.
