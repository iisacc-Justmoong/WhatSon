# `src/app/models/editor/gutter/ContentsGutterLineNumberGeometry.hpp`

## Role

Declares the QML-visible QObject that projects gutter line-number entries from RAW source lines.

## Contract

- Receives RAW editor source text as the backing coordinate space and as the final fallback when block metadata is not
  available.
- Receives `editorGeometryHost` from `ContentsStructuredDocumentFlow.qml`; this host supplies the mounted editor's
  actual source-line rectangles.
- Receives presentation-owned `displayBlocks` and parser-owned `documentBlocks` as source-line metadata. These streams
  annotate rows with resource/block ownership, but RAW physical line starts remain the row-count source.
- Treats `text-group.groupedBlocks`, parser logical grouping, RichText wrapping, and display block fragmentation as
  presentation metadata, not line-number rows.
- Receives `renderedResources` and `editorContentHeight` as refresh inputs only; they do not stretch rows by
  themselves.
- Uses `logicalToSourceOffsets` to map RAW source-line starts onto rendered logical display offsets before sampling
  `editorGeometryHost.lineStartRectangle(...)`.
- Uses `mapTarget` to convert sampled editor points into the gutter item's coordinate space, including scroll offset.
- Keeps `logicalLineStartOffsets` as a compatibility input. It does not change row count.
- Publishes `lineNumberEntries`, a list of `{ lineNumber, y, height, sourceStart, sourceEnd, blockType }` maps consumed
  by `view/contents/Gutter.qml`.
- Keeps published y values cumulative and strictly ordered by RAW source-line order.
- Lets rendered resource rows become tall only when the editor geometry reports the next source line below the rendered
  frame; it never infers that height from total content height.
- Keeps a fallback top inset and line height so standalone Figma frames can render deterministic line-number positions
  even without a mounted live editor.

## Boundary

This class does not parse `.wsnbody` semantics, inspect RichText HTML, own line-number styling, or mutate editor
source. It only converts RAW source line starts, parser/presentation block metadata, and mounted editor geometry into
gutter-coordinate line-number rows.
