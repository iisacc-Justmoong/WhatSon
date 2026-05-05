# `src/app/qml/view/contents/Minimap.qml`

## Role
`Minimap.qml` renders the Figma minimap rail for node `352:8626`.

## Contract
- Imports LVRS and uses LVRS color and metric tokens for row spacing and line height.
- Exposes `rowCount`, `rowWidthRatios`, `scrollDragEnabled`, `horizontalPadding`, and `lineColor` for root-frame binding.
- Emits `scrollDeltaRequested(real deltaY)` while the user drags vertically over the minimap.
- Emits `viewHookRequested(string reason)` through `requestViewHook(reason)`.

## UI
- The token-composed host width is assigned by `ContentsView.qml`.
- The minimap is a clipped column of repeated LVRS hairline rows inset by 8px on the left and right through
  `LV.Theme.gap8`.
- Rail width, row-count policy, and row-specific width-ratio measurement live in `src/app/models/editor/minimap`;
  this file renders the values it receives.
- The default row count is a token-only placeholder. Real design/runtime values are supplied by the host from
  `ContentsMinimapLayoutMetrics`.
- Runtime editor hosts supply one row per visible wrapped editor line, so a single source tag that wraps across two
  displayed text lines renders two minimap rows.
- Runtime editor hosts also expand tall rendered blocks, including resource frames, into the equivalent number of
  line-height rows before binding `rowCount`.
- Each row width follows `rowWidthRatios[index]` when supplied and is resolved against the padded inner rail width.
  Text rows therefore mirror the visible line length in the note editor instead of always occupying the full minimap rail.
- The minimap owns only delta-level drag interaction. It does not know the editor `Flickable`; runtime hosts apply
  `scrollDeltaRequested(...)` as direct viewport movement. Pressing only stores the drag origin, and each drag pixel
  emits the same scroll delta so the minimap does not behave like a slowed scrollbar thumb.
- The minimap does not render a viewport thumb, scrollbar, or current-scroll indicator. It remains a document
  silhouette with an invisible drag surface layered above the rows.
