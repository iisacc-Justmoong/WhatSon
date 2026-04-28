# `src/app/qml/view/content/editor/ContentsMinimapLayer.qml`

## Responsibility

`ContentsMinimapLayer.qml` renders the passive minimap silhouette, viewport thumb, current-line highlight, and drag
scroll interaction.

The component does not own editor geometry. It receives already-resolved numeric viewport/current-line values from the
parent view and paints them without re-entering editor layout state.

## Public Surface

- `minimapVisualRows`: row model for the painted silhouette.
- Structured note rows may now arrive as parser-derived block silhouettes instead of measured per-line editor geometry.
- `minimapBarWidthResolver`, `minimapVisualRowPaintHeightResolver`, `minimapVisualRowPaintYResolver`: paint-only
  callbacks used by the canvas for each row. The width resolver receives the full row object so it can use measured
  `contentWidth` / `contentAvailableWidth` before falling back to `charCount`.
- `minimapSilhouetteHeight`: resolved track-height ceiling from the parent view.
- `minimapScrollable`: whether the viewport thumb should be shown.
- `minimapViewportHeight` / `minimapViewportY`: resolved viewport thumb geometry.
- `minimapCurrentLineHeight` / `minimapCurrentLineWidth` / `minimapCurrentLineY`: resolved current-line highlight
  geometry.
- `scrollToMinimapPositionHandler`: parent-owned scroll callback for click/drag interaction.

## Binding Rules

- The track height clamps against the parent height and the resolved `minimapSilhouetteHeight`; it no longer asks the
  parent for that value through a callback binding.
- Track inset/width and thumb radii now come from `LV.Theme.gap8` and `LV.Theme.scaleMetric(...)`, and the minimap
  colors now resolve from LVRS theme tokens (`accentGray`, `descriptionColor`, `accentTransparent`) instead of local
  hex literals.
- The layer now also exposes an explicit implicit width derived from the track width plus horizontal insets, with a
  56px floor.
  Parent `RowLayout` containers therefore keep a real slot reserved for the minimap instead of opportunistically
  collapsing the right-side column to zero width under sibling layout pressure.
- The viewport thumb and current-line highlight consume numeric snapshot properties directly, not resolver callbacks.
  This breaks the old cycle where the minimap could reopen editor layout computations while binding its own rectangles.
- `minimapScrollable`, `minimapViewportHeight`, and `minimapViewportY` are expected to arrive as parent-owned snapshot
  values, not as live bindings back into `ContentsDisplayView.qml`.
- Only the canvas row paint path still uses resolver callbacks, because those values are paint-time projections from a
  static row model rather than live layout dependencies.
- The canvas opacity treats either `charCount` or measured `contentWidth` as visible content, allowing non-text/visual
  block rows to remain visible without inventing text length.

## Collaborators

- `ContentsDisplayView.qml`: computes all resolved geometry and passes the scroll callback.
- `LV.WheelScrollGuard`: forwards wheel interaction back into the shared editor flickable.

## Regression Checks

- Opening a note should no longer emit repeated binding-loop warnings for minimap `scrollable`, `height`, or `y`.
- The minimap column should remain visibly present on the editor's right edge; expanding sibling editor/panel content
  must not compress the minimap layout slot away.
- Differently sized authored lines should produce differently sized minimap bars that follow the body silhouette from top
  to bottom.
- Dragging or clicking the minimap should still reposition the shared editor viewport.
- The viewport thumb should disappear when the document fits entirely inside the editor viewport.
