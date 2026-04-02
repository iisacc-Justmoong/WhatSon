# `src/app/qml/view/content/editor/ContentsEditorSelectionController.qml`

## Responsibility

`ContentsEditorSelectionController.qml` owns editor selection caching, inline formatting actions, and selection
context-menu dispatch for `ContentsDisplayView.qml`.

The controller keeps source-range bookkeeping separate from the larger editor layout shell so RichText selection and
`.wsnbody` mutation rules no longer live inline inside the main view file.

## Public Contract

- `cachedSelectionStart` / `cachedSelectionEnd`: last valid source-range snapshot.
- `contextMenuSelectionStart` / `contextMenuSelectionEnd`: selection snapshot captured when the context menu opens.
- `contextMenuItems`: menu model for inline formatting actions.
- `selectedEditorRange()`: resolves the current source-text selection span.
- `openEditorSelectionContextMenu(localX, localY)`: opens the LVRS context menu when a non-empty selection exists.
- `wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange)`: wraps the selected source span with the requested
  inline style tag and persists the result.
- `handleSelectionContextMenuEvent(eventName)`: routes menu events through the same formatting mutation path as
  keyboard shortcuts.

## Range Mapping Rules

- The controller first prefers explicit selection offsets from `LV.TextEditor`, then falls back to selected-text
  inference when RichText cursor offsets and source markup offsets diverge.
- `sourceOffsetForLogicalOffset(...)` delegates to `ContentsLogicalTextBridge` when available so rendered plain-text
  offsets can be translated back into source-markup offsets.
- Formatting actions require a non-empty resolved selection range before mutating `.wsnbody`.

## Persistence Rules

- Mutations call `editorSession.markLocalEditorAuthority()` before persistence.
- Immediate persistence goes through `selectionBridge.persistEditorTextForNote(...)` when the contract is available.
- If immediate persistence is unavailable or fails, the controller falls back to
  `editorSession.scheduleEditorPersistence()`.
- The host view still emits `editorTextEdited(...)`; the controller owns the mutation decision but not the broader
  editor-shell lifecycle.
