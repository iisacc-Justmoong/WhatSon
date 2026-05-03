# `src/app/models/editor/display`

## Responsibility

Contains small editor-display support helpers that are not QML view files.

## Current Modules

- `ContentsEditorSurfaceModeSupport.js`
  Decides whether `ContentViewLayout.qml` should mount the note editor surface or the direct resource editor surface.

## Boundary

XML parsing, HTML rendering, and block-object construction are C++ responsibilities under `parser` and `renderer`.
QML view files stay under `src/app/qml/view`.
