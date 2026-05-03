# `src/app/models/editor/minimap/ContentsMinimapLayoutMetrics.cpp`

## Role

Implements the editor minimap layout calculations declared by `ContentsMinimapLayoutMetrics.hpp`.

## Calculation Rules

- Default minimap width follows the injected `buttonMinWidth` LVRS token.
- Hidden minimaps resolve to `gapNone` width instead of relying on QML arithmetic.
- Runtime row count is clamped to at least the stroke-derived minimum unit.
- Design row count remains available for the standalone Figma `view/contents/ContentsView.qml` frame.
