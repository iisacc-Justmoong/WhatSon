# `src/app/models/editor/display`

## Responsibility

This directory holds editor-domain display helpers that are performance-sensitive or safety-sensitive enough to keep out
of QML script bodies.

## Current Modules

- `ContentsDisplayDocumentSourceResolver.*`
  Resolves which RAW note source the editor should present while note selection, snapshot refresh, and editor-session
  binding are converging.
- `ContentsDisplayViewportCoordinator.*`
  Owns line-offset lookup, minimap track math, viewport correction plans, and structured gutter geometry summaries used
  by `ContentsDisplayView.qml`.

## Boundary

- These helpers stay in the editor domain because they operate on editor-owned RAW source, logical-line offsets, and
  note-session state.
- They are intentionally QML-facing `QObject` bridges, but the calculations themselves live in C++ so the view layer
  stays declarative.
