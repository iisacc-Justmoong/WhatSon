# `src/app/models/editor/gutter/ContentsGutterLineNumberGeometry.cpp`

## Role

Implements gutter line-number y-position projection for the runtime editor shell.

## Calculation Rules

- Display line starts are resolved from `logicalLineStartOffsets` when the editor projection supplies them.
- Matching RAW source offsets are resolved from `logicalToSourceOffsets` and passed beside the display offsets, so the
  QML geometry host can measure the logical display-text probe when the RichText overlay is visible, or the RAW text
  layer when native composition hides that overlay.
- Parser-owned `documentBlocks` are consumed only to identify source lines that contain rendered `resource` blocks.
  The model does not parse XML or inspect HTML; resource ownership remains with the editor parser/renderer pipeline.
- Resolved `renderedResources` are consumed as presentation metadata for resource height reconciliation. When a
  resource entry is actually rendered as an image frame, its source span is preferred over the generic document-block
  resource list so unresolved resources or non-image placeholders do not receive image-frame height.
- Display line-start offsets are not converted back to RAW before measurement; the host owns the actual visible editor
  layout decision.
- If logical display offsets are absent, line starts fall back to RAW newline scanning for standalone/design surfaces.
- For each requested line, the model asks the QML editor geometry host for
  `lineStartRectangle(displayPosition, sourcePosition)`.
- When a gutter item is available, the model asks the host to map the editor-local point into the gutter item with
  `mapEditorPointToItem(...)`.
- Mapped y values are not clamped; `Gutter.qml` clips its own content so scrolled-off lines can move above the visible
  gutter instead of collapsing at the top edge.
- If the editor host reports duplicate or non-increasing y samples, the model advances the later line by the fallback
  line height. Distinct line numbers must never overlap in the gutter, even while the display probe is being refreshed.
- Each line-number entry also publishes a `height`. The base height is the gap to the next sampled display line, with
  `fallbackLineHeight` used for the terminal row.
- When the editor reports a larger actual `editorContentHeight` than the sampled logical rows account for, the extra
  height is assigned first to source lines whose resolved resource entries render as image frames, then falls back to
  parser-owned resource block lines when no rendered-resource metadata is available. This keeps image/resource rows in
  the gutter as tall as the corresponding rendered editor body row while leaving unresolved resource placeholders at
  normal text height. If no resource block is known, the extra height is assigned to the terminal row as a conservative
  fallback.
- After resource extra height is assigned, later line-number y positions are rebuilt cumulatively from the adjusted row
  heights. Lines below a rendered resource are therefore pushed by the same vertical amount as the editor body.
- If the live editor geometry is unavailable, the model falls back to `fallbackTopInset + index * fallbackLineHeight`.
- Output is a stable QVariant list of maps, so `Gutter.qml` can render line labels at absolute y positions instead of
  stacking them in a `Column`, and can size each label row from the resolved body-line height.
