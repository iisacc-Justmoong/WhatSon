# `src/app/models/editor/gutter/ContentsGutterLineNumberGeometry.cpp`

## Role

Implements gutter line-number y-position projection for the runtime editor shell.

## Calculation Rules

- Source line starts are resolved from `logicalLineStartOffsets` plus `logicalToSourceOffsets` when the editor
  projection supplies them.
- If logical mapping is absent, source line starts fall back to RAW newline scanning.
- For each requested line, the model asks the QML editor geometry host for `lineStartRectangle(position)`.
- When a gutter item is available, the model asks the host to map the editor-local point into the gutter item with
  `mapEditorPointToItem(...)`.
- Mapped y values are not clamped; `Gutter.qml` clips its own content so scrolled-off lines can move above the visible
  gutter instead of collapsing at the top edge.
- If the live editor geometry is unavailable, the model falls back to `fallbackTopInset + index * fallbackLineHeight`.
- Output is a stable QVariant list of maps, so `Gutter.qml` can render line labels at absolute y positions instead of
  stacking them in a `Column`.
