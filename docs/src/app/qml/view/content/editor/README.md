# `src/app/qml/view/content/editor`

## Scope

Editor-facing QML view components for the center content surface.

## Child Files

- `ContentsDisplaySurfaceHost.qml`
- `ContentsDisplayView.qml`
- `ContentsInlineFormatEditor.qml`
- `ContentsResourceEditorView.qml`
- `ContentsResourceViewer.qml`
- `ContentsStructuredDocumentFlow.qml`

## Current Pipeline

- `ContentsDisplayView.qml` mounts the selected note RAW body into `ContentsEditorSessionController`.
- `ContentsEditorPresentationProjection` converts that RAW body into editor HTML plus renderer-owned block metadata.
- `ContentsStructuredDocumentFlow.qml` consumes `editorSurfaceHtml`, `htmlTokens`, and `normalizedHtmlBlocks`.
- `ContentsInlineFormatEditor.qml` keeps editing on a native `TextEdit.PlainText` buffer while displaying the
  read-side RichText overlay.
- Resource-backed center-surface browsing is handled by `ContentsResourceEditorView.qml` and `ContentsResourceViewer.qml`.

QML in this directory must stay presentation-only. XML parsing, HTML tokenization, block object construction, and RAW
source mutation policy remain in C++ model/renderer objects.
