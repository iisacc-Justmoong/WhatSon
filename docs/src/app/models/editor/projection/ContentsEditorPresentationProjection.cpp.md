# `src/app/models/editor/projection/ContentsEditorPresentationProjection.cpp`

## Responsibility

Implements the QML-facing bridge between RAW editor source and read-side presentation state.

## Pipeline

- `setSourceText(...)` forwards RAW `.wsnbody` text to `ContentsTextFormatRenderer`.
- `ContentsTextFormatRenderer` runs the XML-to-HTML block render pipeline.
- `editorSurfaceHtml`, `htmlTokens`, and `normalizedHtmlBlocks` are republished to QML.
- `ContentsLogicalTextBridge` remains responsible for logical text and line-offset projections.
- RAW cursor projection is published as `logicalCursorPosition` from the model after QML supplies the current
  `sourceCursorPosition`.
- Rendered-surface click and drag selection call coordinate mapping invokables on this projection object. The whole-note
  offset table remains inside `ContentsLogicalTextBridge`; QML does not bind or iterate it.
- `ContentsLogicalTextBridge::logicalToSourceOffsetsChanged` invalidates `logicalCursorPosition`, so note-load and
  reparse turns update caret projection even when the native source cursor offset itself did not change.

## Notes

The object intentionally forwards renderer outputs instead of re-tokenizing in QML. This keeps final editor rendering on
the parser/renderer side and prevents the view layer from rediscovering block boundaries.
