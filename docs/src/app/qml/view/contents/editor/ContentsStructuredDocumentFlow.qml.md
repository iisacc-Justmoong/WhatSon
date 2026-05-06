# `src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml`

## Responsibility

Hosts the note document surface after `ContentsEditorDisplayBackend` has mounted a selected note session.

## Current Contract

- Receives RAW `sourceText` from `ContentsEditorSessionController`.
- Receives resource-resolved `editorSurfaceHtml`, `logicalText`, `projectionSourceText`, `htmlTokens`, and `normalizedHtmlBlocks` from
  the backend-bound note editor chrome in `ContentViewLayout.qml`.
- Mounts `ContentsInlineFormatEditor.qml` as the live `LV.TextEditor` path. The inline editor receives RAW
  `sourceText`, but in rendered mode its native text surface edits the visible logical projection and reports mapped
  RAW cursor/selection offsets back to this host.
- Mounts `ContentsEditorTagInsertionController` and binds the inline editor's tag-management key hook to the
  document flow.
- Handles explicit formatting/body tag shortcuts by asking the tag insertion controller to build the next RAW tag
  insertion payload from the live inline editor buffer, then applying that payload through the same live editor. The
  payload source must not lag behind the focused editor text while parent session bindings catch up.
- Passes projection-ready logical display text into the inline editor's display-geometry probe for cursor and selection
  projection. The flow keeps the last ready logical text while `projectionSourceText` lags behind `sourceText`, so a
  transient parser/render turn cannot fall back to displaying RAW `.wsnbody` tags.
- Passes the current logical cursor position into the inline editor as a projection hint. The host reads
  `editor.sourceCursorPosition`, `editor.sourceSelectionStart`, and `editor.sourceSelectionEnd` back from the inline
  editor so gutter, minimap, formatting, and persistence surfaces keep using RAW `.wsnbody` coordinates.
- Passes the projection-owned `coordinateMapper` object into the inline editor for source-to-rendered selection
  projection; the view layer does not receive the raw logical/source offset table.
- Passes renderer-owned `normalizedHtmlBlocks` through to the inline editor only as selection metadata. Resource
  selection is based on the iiHtmlBlock display block span and painted as one atomic block.
- Keeps the inline editor as the only place where the RichText editor-surface overlay is painted.
- Emits `sourceTextEdited(text)` upward when the user changes the RAW text buffer.
- Tag-management shortcuts emit through the same `sourceTextEdited(text)` path after the live editor buffer is
  updated; they do not serialize RichText back into `.wsnbody`.
- Exposes `normalizedBlocks()` as a compatibility hook for callers that need the renderer-owned block stream.
- Exposes `editorContentHeight`, `editorCursorPosition`, `editorSelectionStart`, `editorSelectionEnd`,
  `editorVisualLineCount`, and `editorVisualLineWidthRatios` to the parent editor host.
- Exposes `editorLogicalGutterRows` to the parent editor host. These rows come from the inline editor's
  `ContentsLineNumberRailMetrics` C++ object and represent logical lines with measured y/height values for the left
  gutter. The cursor and selection offsets let the parent route active-line state to that gutter without adding input
  handling to the rail.
- `editorVisualLineCount` comes from the inline editor's visible wrapped text surface and is the minimap row-count
  input. It includes height-derived row equivalents for tall rendered blocks such as resource frames.
- `editorVisualLineWidthRatios` carries the matching row-width ratios so minimap rows can reflect the visible text
  length of each editor line.
- Exposes `pointRequestsTerminalBodyClick(localX, localY)` and `focusTerminalBodyFromPoint(localX, localY)` for the
  bottom-empty-area accessibility hit target.
- Mounts a transparent left-click `MouseArea` only over the region below the rendered body. The start y follows the
  inline editor's terminal-hit threshold: rendered body height plus one line height and one pixel. Short RichText
  `contentHeight` measurements therefore cannot cover the visible first line or its line-adjacent hit area and steal
  rendered pointer selection from `ContentsInlineFormatEditor`.

## Pipeline Position

This file does not parse XML or infer HTML block boundaries. The pipeline is:

1. RAW `.wsnbody` text is edited.
2. `ContentsEditorPresentationProjection` reparses and renders the snapshot.
3. `ContentsHtmlBlockRenderPipeline` converts the HTML projection through `iiHtmlBlock`.
4. `ContentsEditorDisplayBackend` resolves any resource placeholders through `ContentsBodyResourceRenderer`.
5. `ContentsStructuredDocumentFlow.qml` consumes the final HTML string and block metadata, plus the projection source
   snapshot used to decide whether the logical display text belongs to the current RAW source.

The QML host remains a view surface; RAW mutation and parsing authority stay in C++.
