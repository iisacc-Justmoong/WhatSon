# `src/app/models/editor/display/ContentsDisplayGeometryController.qml`

## Responsibility

Owns QML-runtime geometry scheduling for the editor display host.

## Boundary

- Runs minimap, gutter, and viewport scheduling that depends on `Qt.callLater`.
- Delegates public access through `ContentsDisplayGeometryViewModel`.
- Refreshes current-line minimap width through the measured line-width path when the row model provides
  `contentWidth` / `contentAvailableWidth`.
- Does not persist editor state or render view layout.
