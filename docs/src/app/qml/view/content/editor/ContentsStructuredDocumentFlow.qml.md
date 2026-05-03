# `src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml`

## Responsibility

Hosts the note document surface after `ContentsDisplayView.qml` has mounted a selected note session.

## Current Contract

- Receives RAW `sourceText` from `ContentsEditorSessionController`.
- Receives `editorSurfaceHtml`, `htmlTokens`, and `normalizedHtmlBlocks` from
  `ContentsEditorPresentationProjection`.
- Mounts `ContentsInlineFormatEditor.qml` as the live `LV.TextEditor` path.
- Emits `sourceTextEdited(text)` upward when the user changes the RAW text buffer.
- Exposes `normalizedBlocks()` as a compatibility hook for callers that need the renderer-owned block stream.

## Pipeline Position

This file does not parse XML or infer HTML block boundaries. The pipeline is:

1. RAW `.wsnbody` text is edited.
2. `ContentsEditorPresentationProjection` reparses and renders the snapshot.
3. `ContentsHtmlBlockRenderPipeline` converts the HTML projection through `iiHtmlBlock`.
4. `ContentsStructuredDocumentFlow.qml` consumes the final HTML string and block metadata.

The QML host remains a view surface; RAW mutation and parsing authority stay in C++.
