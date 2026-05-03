# `src/app/models/editor/gutter/ContentsGutterLineNumberGeometry.hpp`

## Role

Declares the QML-visible QObject that projects gutter line-number entries from editor line geometry.

## Contract

- Receives the live editor geometry host from `ContentsStructuredDocumentFlow.qml`.
- Receives the gutter QML item as `mapTarget`, so editor-local line y values can be mapped into gutter coordinates.
- Receives RAW editor source text, logical display line-start offsets, and logical-to-source offset mapping from the
  editor projection.
- Passes both logical display offsets and their matching RAW source offsets to the live editor geometry host; the host
  chooses the coordinate that matches the currently visible editor layer.
- Falls back to RAW newline scanning when logical display offsets are not supplied.
- Publishes `lineNumberEntries`, a list of `{ lineNumber, y }` maps consumed by `contents/Gutter.qml`.
- Keeps a fallback top inset and line height so standalone Figma frames can render deterministic line-number positions
  even without a mounted live editor.

## Boundary

This class does not parse `.wsnbody` semantics, own line-number styling, or mutate editor source. It only converts
display line starts plus live editor rectangle data into gutter-coordinate y positions.
