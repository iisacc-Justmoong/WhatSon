# `src/app/models/editor/gutter/ContentsGutterLineNumberGeometry.hpp`

## Role

Declares the QML-visible QObject that projects gutter line-number entries from editor document blocks.

## Contract

- Receives RAW editor source text as the backing coordinate space and as the final fallback when block metadata is not
  available.
- Receives presentation-owned `displayBlocks` from `ContentsEditorPresentationProjection.normalizedHtmlBlocks`; this is
  the flattened block stream that the editor actually renders.
- Receives parser-owned `documentBlocks` from `ContentsStructuredBlockRenderer.renderedDocumentBlocks`; this is the
  logical structured block stream, including grouped text blocks and atomic resource blocks.
- Builds gutter rows from both block streams. The richer stream becomes primary and any missing source ranges from the
  secondary stream are merged in, so resource-only display refreshes cannot collapse the gutter to one row.
- `text-group` entries with `groupedBlocks` are expanded into their logical children, while each `resource` entry
  remains one atomic gutter row.
- Receives `renderedResources` from `ContentsBodyResourceRenderer` as the presentation-side resource metadata used to
  distinguish image-frame resources from unresolved or non-image placeholders.
- Keeps `editorGeometryHost`, `mapTarget`, `logicalLineStartOffsets`, and `logicalToSourceOffsets` as inert
  compatibility inputs, but y-position calculation does not sample editor geometry.
- Publishes `lineNumberEntries`, a list of `{ lineNumber, y, height, sourceStart, sourceEnd, blockType }` maps consumed
  by `view/contents/Gutter.qml`.
- Keeps published y values cumulative and strictly ordered by visible document-block order.
- Reconciles actual editor content height against rendered image-resource source ranges first, then parser-owned
  resource source ranges as a fallback, so a visible framed resource row can own the same vertical height in the gutter
  that it occupies in the editor body without inflating unresolved resource placeholders.
- Keeps a fallback top inset and line height so standalone Figma frames can render deterministic line-number positions
  even without a mounted live editor.

## Boundary

This class does not parse `.wsnbody` semantics, inspect RichText HTML, own line-number styling, or mutate editor
source. It only converts parser/presentation block streams, RAW source spans, and resolved resource presentation
metadata into gutter-coordinate line-number rows.
