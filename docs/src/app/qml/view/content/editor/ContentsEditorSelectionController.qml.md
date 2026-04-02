# `src/app/qml/view/content/editor/ContentsEditorSelectionController.qml`

## Responsibility

`ContentsEditorSelectionController.qml` owns editor selection resolution, inline formatting actions, and selection
context-menu dispatch for `ContentsDisplayView.qml`.

The controller keeps source-range bookkeeping separate from the larger editor layout shell so RichText selection and
`.wsnbody` mutation rules no longer live inline inside the main view file.

The controller talks to the editor through the stable `contentEditor` contract (`selectionSnapshot()`, `getText(...)`,
`getFormattedText(...)`, `cursorPosition`, `selectionStart`, `selectionEnd`, `selectedText`, `editorItem`,
`inputItem`) so the host view can swap the concrete editor surface without rewriting formatting logic.

## Public Contract

- `contextMenuSelectionStart` / `contextMenuSelectionEnd`: selection snapshot captured when the context menu opens.
- `contextMenuItems`: menu model for inline formatting actions. The top item is `Plain`, which clears inline
  formatting from the current selection.
- `normalizeEditorSurfaceTextToSource(...)`: converts live RichText editor output back into canonical `.wsnbody`
  source tags.
- `selectedEditorRange()`: resolves the current editor-surface selection span.
- `openEditorSelectionContextMenu(localX, localY)`: opens the LVRS context menu when a non-empty selection exists.
- `wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange)`: wraps the exact rendered RichText fragment from the
  live editor with the requested inline style tag and persists the canonicalized result.
- `handleSelectionContextMenuEvent(eventName)`: routes menu events through the same formatting mutation path as
  keyboard shortcuts.

## Range Mapping Rules

- The controller first prefers the live `TextEdit` selection snapshot exposed by `contentEditor.selectionSnapshot()`.
- Explicit numeric offsets are only accepted when `contentEditor.getText(start, end)` matches the currently highlighted
  `selectedText`.
- When numeric offsets disagree with the actual highlighted substring, the controller falls back to selected-text
  inference against the live editor plain-text surface returned by `contentEditor.getText(...)`, not against raw
  `.wsnbody` source text.
- Shortcut-triggered wraps capture the resolved editor-surface selection range first, then queue the wrap one event-loop turn
  later. The queued mutation therefore reuses the original selection snapshot instead of re-reading a possibly collapsed
  post-shortcut selection.
- The controller no longer slices `.wsnbody` directly for inline-format actions.
- It captures the current RichText surface from the live editor and delegates the actual selection mutation to
  `ContentsTextFormatRenderer.applyInlineStyleToSelectionSource(...)`, which uses `QTextDocument/QTextCursor`.
- The context menu exposes `Plain` as a first-class formatting action. It routes through the same renderer path and
  clears all supported inline styles from the selected range before persisting canonical `.wsnbody`.
- Reapplying the same inline style to a fully formatted selection now clears that selection back to plain text instead
  of stacking duplicate RichText spans/source tags.
- The controller still persists canonical source tags (`<bold>`, `<italic>`, ...) directly; RichText HTML remains an
  editor-surface projection only.
- Formatting actions require a non-empty resolved selection range before mutating `.wsnbody`.

## Persistence Rules

- Mutations call `editorSession.markLocalEditorAuthority()` before persistence.
- Immediate persistence goes through `selectionBridge.persistEditorTextForNote(...)` when the contract is available.
- The save pipeline canonicalizes the edited RichText surface back into `.wsnbody` source tags, so shortcut/context
  formatting persists as semantic tags such as `<bold>...</bold>` and `<italic>...</italic>` instead of raw span CSS.
- A successful immediate save clears the local-authority/pending-save window so the model can immediately feed the
  canonical inline-tag source back into the editor normalization path.
- If immediate persistence is unavailable or fails, the controller falls back to
  `editorSession.scheduleEditorPersistence()`.
- The host view still emits `editorTextEdited(...)`; the controller owns the mutation decision but not the broader
  editor-shell lifecycle.

## Regression Checks

- Re-selecting a different span and pressing `Cmd/Ctrl+B` / `I` / `U` / `Shift+X` / `Shift+E` should wrap the latest
  visible selection from the live `TextEdit`, not an older fallback snapshot.
- Formatting should apply to the exact rendered fragment under the current selection even when the note body already
  contains inline tags around nearby text.
- Applying the same shortcut twice to an already formatted selection should restore that selection to plain text.
- Choosing `Plain` from the context menu should remove all inline formatting tags from the selected source range.
- Shortcut and window-level accelerator paths should coalesce into one queued wrap request per note/tag pair, avoiding
  duplicate formatting from the same key chord.
