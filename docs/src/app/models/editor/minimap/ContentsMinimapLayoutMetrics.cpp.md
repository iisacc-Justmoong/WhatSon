# `src/app/models/editor/minimap/ContentsMinimapLayoutMetrics.cpp`

## Role

Implements the editor minimap layout calculations declared by `ContentsMinimapLayoutMetrics.hpp`.

## Calculation Rules

- Default minimap width follows the injected `buttonMinWidth` LVRS token.
- Hidden minimaps resolve to `gapNone` width instead of relying on QML arithmetic.
- Runtime row count is the injected `visualLineCount` from `ContentsEditorVisualLineMetrics`, clamped to at least the
  stroke-derived minimum unit.
- Wrapped editor text contributes one minimap row per visible wrapped line, and tall rendered blocks contribute rows by
  dividing their visible content height by the editor line height before the value reaches this model.
- Design row count remains available for the standalone Figma `view/contents/ContentsView.qml` frame.
