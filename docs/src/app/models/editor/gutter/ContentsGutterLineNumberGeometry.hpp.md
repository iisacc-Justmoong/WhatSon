# `src/app/models/editor/gutter/ContentsGutterLineNumberGeometry.hpp`

## Role

Declares the QML-visible QObject that projects gutter line-number entries from editor line geometry.

## Contract

- Receives the live editor geometry host from `ContentsStructuredDocumentFlow.qml`.
- Receives the gutter QML item as `mapTarget`, so editor-local line y values can be mapped into gutter coordinates.
- Receives RAW editor source text, logical display line-start offsets, and logical-to-source offset mapping from the
  editor projection.
- Receives parser-owned `documentBlocks` only as source-range metadata for resource-line height reconciliation.
- Receives `renderedResources` from `ContentsBodyResourceRenderer` as the presentation-side resource metadata used to
  distinguish image-frame resources from unresolved or non-image placeholders.
- Passes both logical display offsets and their matching RAW source offsets to the live editor geometry host; the host
  chooses the coordinate that matches the currently visible editor layer. When the RichText overlay is visible, that
  host measures a logical plain-text display probe instead of the HTML source string, so hidden tags cannot collapse
  early line-number positions.
- Falls back to RAW newline scanning when logical display offsets are not supplied.
- Publishes `lineNumberEntries`, a list of `{ lineNumber, y, height }` maps consumed by `view/contents/Gutter.qml`.
- Keeps published y values strictly increasing for consecutive line numbers to prevent line-number overlap while live
  editor geometry is settling.
- Reconciles actual editor content height against rendered image-resource source ranges first, then parser-owned
  resource source ranges as a fallback, so a visible framed resource row can own the same vertical height in the gutter
  that it occupies in the editor body without inflating unresolved resource placeholders.
- Keeps a fallback top inset and line height so standalone Figma frames can render deterministic line-number positions
  even without a mounted live editor.

## Boundary

This class does not parse `.wsnbody` semantics, inspect RichText HTML, own line-number styling, or mutate editor
source. It only converts parser-provided block/source metadata, resolved resource presentation metadata, and live editor
rectangle data into gutter-coordinate line-number rows.
