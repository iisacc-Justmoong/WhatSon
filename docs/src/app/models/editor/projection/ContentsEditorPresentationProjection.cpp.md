# `src/app/models/editor/projection/ContentsEditorPresentationProjection.cpp`

## Responsibility

Implements the QML-facing bridge between RAW editor source and read-side presentation state.

## Pipeline

- `setSourceText(...)` forwards RAW `.wsnbody` text to `ContentsTextFormatRenderer`.
- `ContentsTextFormatRenderer` runs the XML-to-HTML block render pipeline.
- `editorSurfaceHtml`, `htmlTokens`, and `normalizedHtmlBlocks` are republished to QML.
- `ContentsLogicalTextBridge` remains responsible for logical text and line-offset projections.
- RAW cursor projection is forwarded through `ContentsLogicalTextBridge::logicalOffsetForSourceOffset(...)` so QML
  caret painting can resolve positions inside hidden inline tags against the complete source snapshot.

## Notes

The object intentionally forwards renderer outputs instead of re-tokenizing in QML. This keeps final editor rendering on
the parser/renderer side and prevents the view layer from rediscovering block boundaries.
