# `src/app/models/editor/gutter/ContentsGutterLineNumberGeometry.cpp`

## Role

Implements gutter line-number y-position projection for the runtime editor shell.

## Calculation Rules

- Line-number rows are derived from RAW source physical line starts, matching ordinary text-editor gutter behavior.
- Line-number y positions are sampled from the mounted editor geometry whenever `editorGeometryHost` is available.
- `displayBlocks` carries the flattened, actually rendered block stream from
  `ContentsEditorPresentationProjection.normalizedHtmlBlocks`.
- `documentBlocks` carries the parser-owned logical structured block stream from
  `ContentsStructuredBlockRenderer.renderedDocumentBlocks`.
- Parser/display block streams annotate source-line rows with block type and resource ownership. They do not define the
  row count, and `text-group.groupedBlocks` never creates synthetic gutter rows.
- If no source text is available, the model falls back to one empty source line at `fallbackTopInset`.
- `lineNumberEntries` are built in source-line order. The y value for each row is the editor-reported rectangle for that
  source line's start offset, with cumulative fallback positions only when live geometry is unavailable.
- The model does not parse XML or inspect HTML; resource and block ownership remain with the editor parser/renderer
  pipeline.
- `renderedResources` and `editorContentHeight` remain refresh inputs, but they do not assign extra document height to
  any gutter row. Resource-frame height must come from the mounted editor geometry itself.
- `editorGeometryHost.lineStartRectangle(logicalOffset, sourceOffset)` is the preferred y source. Each row keeps both
  its RAW source span and its HTML/logical line span. The model first uses `logicalLineStartOffsets[lineIndex]`,
  matching code-editor line-block lookup; only missing entries fall back to `logicalToSourceOffsets` source-offset
  inversion.
- If `mapTarget` is provided, sampled editor points are converted into that target item's coordinate space through
  `editorGeometryHost.mapEditorPointToItem(...)`. This keeps the gutter aligned with the scrolled viewport instead of
  the unscrolled document origin.
- Row count stays source-line based even when logical line-start offsets are used for the visible y lookup.
- Each line-number entry publishes the source row, logical span, and geometry result:
  `rawLineIndex`, `rawSourceStart`, `rawSourceEnd`, `logicalStartOffset`, `logicalEndOffset`, `geometryY`,
  `geometryHeight`, and `geometrySampled`. The public `y`/`height` values are the resolved geometry row box.
- With editor geometry, each row height is the distance to the next sampled source-line y position. The final row uses
  the sampled rectangle height, so different numbered rows can legitimately have different heights.
- No post-pass distributes remaining `editorContentHeight` into resource or terminal rows. Rows below a rendered
  resource stay aligned only because their sampled editor y positions already include the rendered resource frame.
- Output is a stable QVariant list of maps, so `Gutter.qml` can render line labels at absolute y positions instead of
  stacking them in a `Column`, and can size each label row from the resolved body-line height.
