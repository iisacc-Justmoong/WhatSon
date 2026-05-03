# `src/app/models/editor/gutter/ContentsGutterLineNumberGeometry.cpp`

## Role

Implements gutter line-number y-position projection for the runtime editor shell.

## Calculation Rules

- Display line starts are resolved from `logicalLineStartOffsets` when the editor projection supplies them.
- Matching RAW source offsets are resolved from `logicalToSourceOffsets` and passed beside the display offsets, so the
  QML geometry host can measure either the RichText display layer or the RAW text layer depending on what is currently
  visible.
- Display line-start offsets are not converted back to RAW before measurement; the host owns the actual visible editor
  layout decision.
- If logical display offsets are absent, line starts fall back to RAW newline scanning for standalone/design surfaces.
- For each requested line, the model asks the QML editor geometry host for
  `lineStartRectangle(displayPosition, sourcePosition)`.
- When a gutter item is available, the model asks the host to map the editor-local point into the gutter item with
  `mapEditorPointToItem(...)`.
- Mapped y values are not clamped; `Gutter.qml` clips its own content so scrolled-off lines can move above the visible
  gutter instead of collapsing at the top edge.
- If the live editor geometry is unavailable, the model falls back to `fallbackTopInset + index * fallbackLineHeight`.
- Output is a stable QVariant list of maps, so `Gutter.qml` can render line labels at absolute y positions instead of
  stacking them in a `Column`.
