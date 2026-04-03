# `src/app/qml/view/content/editor/ContentsMinimapLayer.qml`

## Responsibility

`ContentsMinimapLayer.qml` renders the passive minimap silhouette, viewport thumb, current-line highlight, and drag
scroll interaction.

The component does not own editor geometry. It receives already-resolved numeric viewport/current-line values from the
parent view and paints them without re-entering editor layout state.

## Public Surface

- `minimapVisualRows`: row model for the painted silhouette.
- `minimapBarWidthResolver`, `minimapVisualRowPaintHeightResolver`, `minimapVisualRowPaintYResolver`: paint-only
  callbacks used by the canvas for each row.
- `minimapSilhouetteHeight`: resolved track-height ceiling from the parent view.
- `minimapScrollable`: whether the viewport thumb should be shown.
- `minimapViewportHeight` / `minimapViewportY`: resolved viewport thumb geometry.
- `minimapCurrentLineHeight` / `minimapCurrentLineWidth` / `minimapCurrentLineY`: resolved current-line highlight
  geometry.
- `scrollToMinimapPositionHandler`: parent-owned scroll callback for click/drag interaction.

## Binding Rules

- The track height clamps against the parent height and the resolved `minimapSilhouetteHeight`; it no longer asks the
  parent for that value through a callback binding.
- The viewport thumb and current-line highlight consume numeric snapshot properties directly, not resolver callbacks.
  This breaks the old cycle where the minimap could reopen editor layout computations while binding its own rectangles.
- `minimapScrollable`, `minimapViewportHeight`, and `minimapViewportY` are expected to arrive as parent-owned snapshot
  values, not as live bindings back into `ContentsDisplayView.qml`.
- Only the canvas row paint path still uses resolver callbacks, because those values are paint-time projections from a
  static row model rather than live layout dependencies.

## Collaborators

- `ContentsDisplayView.qml`: computes all resolved geometry and passes the scroll callback.
- `LV.WheelScrollGuard`: forwards wheel interaction back into the shared editor flickable.

## Regression Checks

- Opening a note should no longer emit repeated binding-loop warnings for minimap `scrollable`, `height`, or `y`.
- Dragging or clicking the minimap should still reposition the shared editor viewport.
- The viewport thumb should disappear when the document fits entirely inside the editor viewport.
