# `src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml`

## Responsibility

Hosts the note document surface after `ContentsDisplayView.qml` has mounted a selected note session.

## Current Contract

- Receives RAW `sourceText` from `ContentsEditorSessionController`.
- Receives resource-resolved `editorSurfaceHtml`, `logicalText`, `htmlTokens`, and `normalizedHtmlBlocks` from
  `ContentsDisplayView.qml`.
  The upstream `editorSurfaceHtml` starts in `ContentsEditorPresentationProjection` and is then passed through
  `ContentsInlineResourcePresentationController` so resource placeholder comments become actual inline resource
  HTML before this view surface paints it.
- Mounts `ContentsInlineFormatEditor.qml` as the live `LV.TextEditor` path.
- Passes `logicalText` into the inline editor's display-geometry probe for cursor, selection, and pointer mapping.
- Passes the current logical cursor position into the inline editor so its overlay cursor is projected from visible
  text geometry while the authoritative edit buffer remains RAW source text.
- Passes the projection-owned `logicalToSourceOffsets` table into the inline editor. Surface clicks are resolved from
  visible logical text positions and then converted back to RAW offsets, so clicking rendered prose or a rendered
  resource placeholder does not place the cursor according to hidden RAW tag glyph positions.
- Passes renderer-owned `normalizedHtmlBlocks` through to the inline editor only as selection metadata. Resource
  selection is then based on the iiHtmlBlock display block span and painted as one atomic block, while gutter y/height
  remains responsible for the rendered resource's vertical size.
- Keeps the inline editor as the only place where the RichText editor-surface overlay is painted. The flow supplies
  fresh `editorSurfaceHtml` on every committed RAW source change; it does not add a second preview path while typing.
- Emits `sourceTextEdited(text)` upward when the user changes the RAW text buffer.
- Exposes `normalizedBlocks()` as a compatibility hook for callers that need the renderer-owned block stream.
- Exposes `editorContentHeight` and `editorCursorPosition` to the parent editor host.
- Exposes `lineStartRectangle(position, sourcePosition)` and `mapEditorPointToItem(...)` as the narrow mounted-editor
  geometry hooks used by the gutter sampler. The C++ gutter model still owns source-line row creation; this flow only
  reports visible editor coordinates.
- Exposes `pointRequestsTerminalBodyClick(localX, localY)` and `focusTerminalBodyFromPoint(localX, localY)` for the
  bottom-empty-area accessibility hit target. A point below the rendered editor body is treated like a click at the
  RAW body end, while points over existing body content are passed through to the native editor/link path.
- Mounts a transparent left-click `MouseArea` only over the region below `editorContentHeight`. It no longer covers
  existing body content, so ordinary text clicks, cursor movement, selection, and typing stay on the native editor
  path while the bottom empty area still focuses the RAW body end.

## Pipeline Position

This file does not parse XML or infer HTML block boundaries. The pipeline is:

1. RAW `.wsnbody` text is edited.
2. `ContentsEditorPresentationProjection` reparses and renders the snapshot.
3. `ContentsHtmlBlockRenderPipeline` converts the HTML projection through `iiHtmlBlock`.
4. `ContentsDisplayView.qml` resolves any resource placeholders through `ContentsBodyResourceRenderer`.
5. `ContentsStructuredDocumentFlow.qml` consumes the final HTML string and block metadata.

The QML host remains a view surface; RAW mutation and parsing authority stay in C++.
