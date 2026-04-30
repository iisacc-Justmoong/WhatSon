# `src/app/models/editor/display/ContentsDisplayGeometryController.qml`

## Responsibility

Owns QML-runtime geometry scheduling for the editor display host.

## Boundary

- Runs minimap, gutter, and viewport scheduling that depends on `Qt.callLater`.
- Delegates public access through `ContentsDisplayGeometryController`.
- Refreshes current-line minimap width through the row model's resolved width payload.
  For structured notes that row model is now parser/block derived rather than sampled from every live text row.
- Does not persist editor state or render view layout.
