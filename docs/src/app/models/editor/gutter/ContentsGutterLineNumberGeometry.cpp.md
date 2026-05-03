# `src/app/models/editor/gutter/ContentsGutterLineNumberGeometry.cpp`

## Role

Implements gutter line-number y-position projection for the runtime editor shell.

## Calculation Rules

- Line-number y positions are derived from editor document block order, not from live editor geometry samples.
- `displayBlocks` carries the flattened, actually rendered block stream from
  `ContentsEditorPresentationProjection.normalizedHtmlBlocks`.
- `documentBlocks` carries the parser-owned logical structured block stream from
  `ContentsStructuredBlockRenderer.renderedDocumentBlocks`.
- The model uses the richer available stream as the primary block stream, then merges missing ranges from the secondary
  stream. This allows a visible resource block from `displayBlocks` and structured text rows from `documentBlocks` to
  coexist during parser/projection refresh.
- Flattened `text-group.groupedBlocks` are expanded into child logical rows, each `resource` entry stays one atomic row,
  and RAW newline scanning is used only as a fallback when no block stream is available.
- `lineNumberEntries` are built in visible document-block order. The y value for each row is the cumulative height of
  every previous block row, starting at `fallbackTopInset`.
- The model does not parse XML or inspect HTML; resource and block ownership remain with the editor parser/renderer
  pipeline.
- Resolved `renderedResources` are consumed as presentation metadata for resource height reconciliation. When a
  resource entry is actually rendered as an image frame, its source span is preferred over the generic document-block
  resource list so unresolved resources or non-image placeholders do not receive image-frame height.
- `editorGeometryHost`, `mapTarget`, `logicalLineStartOffsets`, and `logicalToSourceOffsets` remain accepted inputs for
  compatibility with existing QML surfaces, but the model intentionally does not invoke geometry hooks for y mapping.
- Each line-number entry also publishes a `height`. The base height is `fallbackLineHeight`.
- When the editor reports a larger actual `editorContentHeight` than the fallback RAW rows account for, the extra
  height is assigned first to source lines whose resolved resource entries render as image frames, then falls back to
  parser-owned resource block lines when no rendered-resource metadata is available. This keeps image/resource rows in
  the gutter as tall as the corresponding rendered editor body row while leaving unresolved resource placeholders at
  normal text height. If no resource block is known, the extra height is assigned to the terminal row as a conservative
  fallback.
- After resource extra height is assigned, later line-number y positions are rebuilt cumulatively from the adjusted
  block-row heights. Rows below a rendered resource are therefore pushed by the same vertical amount as the editor body.
- If no block metadata or source text is available, the model falls back to one line at `fallbackTopInset`.
- Output is a stable QVariant list of maps, so `Gutter.qml` can render line labels at absolute y positions instead of
  stacking them in a `Column`, and can size each label row from the resolved body-line height.
