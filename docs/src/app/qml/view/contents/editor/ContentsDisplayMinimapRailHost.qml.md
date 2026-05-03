# `src/app/qml/view/contents/editor/ContentsDisplayMinimapRailHost.qml`

## Responsibility

Composes the note-display minimap rail for the unified editor display host.

## Boundary

- Owns only minimap view bindings into `ContentsMinimapLayer`.
- Delegates minimap geometry math to the existing display viewport/geometry coordinators through the host contract.
- Passes the full minimap row object into `minimapLineBarWidth(...)` so sampled editor line widths drive the silhouette
  before character-count fallback is considered.
- Keeps repaint capability on the visual layer so display controllers can request a canvas refresh without knowing the
  internal minimap component tree.
