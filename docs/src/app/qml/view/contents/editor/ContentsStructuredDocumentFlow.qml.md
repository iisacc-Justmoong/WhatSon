# `src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml`

## Responsibility

Hosts the note document surface after `ContentsEditorDisplayBackend` has mounted a selected note session.

## Current Contract

- Receives RAW `sourceText` from `ContentsEditorSessionController`.
- Receives parser-owned `documentBlocks`, unresolved `editorSurfaceHtml`, `resourceVisualBlocks`, `logicalText`,
  `projectionSourceText`, and `normalizedHtmlBlocks` from the backend-bound note editor chrome in
  `ContentViewLayout.qml`.
- Receives `paperPaletteEnabled` and a paper text color from `ContentViewLayout.qml` when the parent Page/Print renderer
  has selected the paper surface.
- Mounts `ContentsInlineFormatEditor.qml` as the live `LV.TextEditor` path. The inline editor receives RAW
  `sourceText`, but in rendered mode its native text surface edits the visible logical projection and reports mapped
  RAW cursor/selection offsets back to this host.
- Mounts `ContentsEditorTagInsertionController` for shortcut-to-tag resolution and
  `ContentsEditorTagMutationBuilder` for shortcut-independent RAW tag payload generation, then binds the inline
  editor's tag-management key hook to the document flow.
- Passes `shortcutPlatformName` into the inline editor so tests and platform shells can verify native-to-standard
  Command/Ctrl shortcut compensation without duplicating shortcut policy in the document flow.
- Handles explicit formatting/body tag shortcuts by asking the shortcut controller for a canonical tag name, then
  asking the mutation builder to build the next RAW tag insertion payload from the live inline editor buffer. The
  payload source must not lag behind the focused editor text while parent session bindings catch up.
- Passes projection-ready logical display text into the inline editor's display-geometry probe for pointer hit testing
  and visible-logical selection mapping. The flow keeps the last ready logical text and editor-surface HTML while
  `projectionSourceText` lags behind `sourceText`, so a transient parser/render turn cannot fall back to displaying
  RAW `.wsnbody` tags or plain logical text.
- Reads `editor.sourceCursorPosition`, `editor.sourceSelectionStart`, and `editor.sourceSelectionEnd` back from the
  inline editor so host chrome, formatting, and persistence surfaces keep using RAW `.wsnbody` coordinates. Metric row
  calculation itself stays in `ContentViewLayout.qml`.
- Passes the projection-owned `coordinateMapper` object into the inline editor for visible-logical to RAW selection
  mapping; the view layer does not receive the raw logical/source offset table.
- Passes parser-owned `documentBlocks` and resolved `resourceVisualBlocks` through to the inline editor so resource
  selection and geometry use explicit atomic visual blocks. The `editorSurfaceHtml` it receives from
  `ContentViewLayout.qml` is already flow-corrected from renderer tokens and `resourceVisualBlocks` by the C++
  display backend. Renderer-owned `normalizedHtmlBlocks` remain compatibility metadata for HTML projection spans.
- Keeps the inline editor as the only place where the RichText editor-surface overlay is painted.
- In Page/Print mode the HTML is already paper-palette corrected by `ContentsEditorDisplayBackend`; this flow only
  forwards the paper text color and bounded paper width into the live inline editor surface.
- Emits `sourceTextEdited(text)` upward when the user changes the RAW text buffer.
- Tag-management shortcuts emit through the same `sourceTextEdited(text)` path after the live editor buffer is
  updated; they do not serialize RichText back into `.wsnbody`.
- Exposes `normalizedBlocks()` as a compatibility hook for callers that need the renderer-owned block stream.
- Exposes `editorContentHeight`, `editorCursorPosition`, `editorSelectionStart`, `editorSelectionEnd`, and
  `editorRenderedOverlayVisible` to the parent editor host.
- Does not relay gutter or minimap metrics from the inline editor. `ContentViewLayout.qml` owns independent metric
  probes and binds their outputs directly to `ContentsLineNumberRail.qml`, `Minimap.qml`, and
  `ContentsMinimapLayoutMetrics`.
- Exposes `pointRequestsTerminalBodyClick(localX, localY)` and `focusTerminalBodyFromPoint(localX, localY)` for the
  bottom-empty-area accessibility hit target.
- Mounts a transparent left-click `MouseArea` only over the region below the rendered body. The start y follows the
  inline editor's terminal-hit threshold: rendered body height, excluding the editor's legacy bottom inset, plus one
  line height and one pixel. Short RichText `contentHeight` measurements therefore cannot cover the visible first line
  or its line-adjacent hit area and steal rendered pointer selection from `ContentsInlineFormatEditor`.

## Pipeline Position

This file does not parse XML or infer HTML block boundaries. The pipeline is:

1. RAW `.wsnbody` text is edited.
2. `ContentsEditorPresentationProjection` reparses and renders the snapshot.
3. `ContentsHtmlBlockRenderPipeline` converts text/semantic blocks through `iiHtmlBlock` and leaves resource blocks as
   atomic document slots.
4. `ContentsEditorDisplayBackend` forwards parser-owned document blocks to `ContentsBodyResourceRenderer` and asks
   `ContentsInlineResourcePresentationController` for structured resource visual blocks.
5. `ContentsStructuredDocumentFlow.qml` consumes the flow-corrected editor HTML string, resource visual blocks,
   parser document blocks, and the projection source snapshot used to decide whether the logical display text belongs
   to the current RAW source.

The QML host remains a view surface; RAW mutation and parsing authority stay in C++.
