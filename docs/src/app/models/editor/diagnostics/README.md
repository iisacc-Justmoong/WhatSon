# `src/app/models/editor/diagnostics`

## Responsibility
Owns editor-domain diagnostic support that can be shared by view hosts, controllers, and model-facing QML helpers.

## Current Modules
- `ContentsEditorDebugTrace.js`
  Formats compact debug trace strings for editor QML objects without leaving diagnostics utilities inside the view
  directory.

## Boundary
- Diagnostics helpers must not own editor state or mutation policy.
- View QML may import diagnostics helpers, but visual layout remains under `src/app/qml/view/content/editor`.
