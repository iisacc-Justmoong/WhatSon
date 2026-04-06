# `src/app/qml/view/content/editor/ContentsEditorSelectionController.qml`

## Responsibility

`ContentsEditorSelectionController.qml` owns editor selection resolution, inline formatting actions, and selection
context-menu dispatch for `ContentsDisplayView.qml`.

The controller keeps source-range bookkeeping separate from the larger editor layout shell so RichText selection and
`.wsnbody` mutation rules no longer live inline inside the main view file.

Ordinary typing is intentionally out of scope for this controller and now lives in
`ContentsEditorTypingController.qml`.

The controller talks to the editor through the stable `contentEditor` contract (`selectionSnapshot()`, `getText(...)`,
`getFormattedText(...)`, `cursorPosition`, `selectionStart`, `selectionEnd`, `selectedText`, `editorItem`,
`inputItem`) so the host view can swap the concrete editor surface without rewriting formatting logic.

It also owns keyboard-driven markdown block toggles for the list types the renderer currently supports:
- unordered list
- ordered list
- task/checklist shortcuts are intentionally absent because the current renderer/source pipeline does not expose a
  dedicated task-list block contract

## Public Contract

- `contextMenuSelectionStart` / `contextMenuSelectionEnd`: selection snapshot captured when the context menu opens.
- `contextMenuItems`: menu model for inline formatting actions. The top item is `Plain`, which clears inline
  formatting from the current selection.
- `selectedEditorRange()`: resolves the current editor-surface selection span.
- `openEditorSelectionContextMenu(localX, localY)`: opens the LVRS context menu when a non-empty selection exists.
- `wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange)`: applies the requested inline style to canonical
  `.wsnbody` source using the resolved logical editor selection range and persists the canonicalized result.
- `handleSelectionContextMenuEvent(eventName)`: routes menu events through the same formatting mutation path as
  keyboard shortcuts.
- `queueMarkdownListMutation(listKind)`: captures the current selection/cursor snapshot and toggles markdown list
  prefixes on the touched logical lines one event-loop turn later.

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
- Markdown list shortcuts follow the same queued-snapshot rule, but operate on whole logical lines instead of requiring
  a non-empty text selection.
- A collapsed cursor toggles only the current logical line.
- A non-empty selection toggles every touched non-empty logical line in the captured range.
- `Cmd+Shift+8` on macOS and `Alt+Shift+8` on Windows/Linux toggle an unordered markdown list and write canonical `- `
  source prefixes.
- `Cmd+Shift+7` on macOS and `Alt+Shift+7` on Windows/Linux toggle an ordered markdown list and write canonical `1. `
  / `2. ` source prefixes.
- Reapplying the same list shortcut to lines that are already uniformly in that list form removes the corresponding
  list markers instead of stacking duplicates.
- Mixed-line conversions are canonicalized:
  - unordered toggle converts ordered/plain lines into canonical `- ` items
  - ordered toggle converts unordered/plain lines into canonical `1. ` / `2. ` / `3. ` items
- The controller no longer mutates the live markdown-rendered RichText surface directly for shortcut formatting.
- It delegates to `ContentsTextFormatRenderer.applyInlineStyleToLogicalSelectionSource(...)`, which builds a
  markdown-neutral source-editing surface from the canonical `.wsnbody` text and applies `QTextDocument/QTextCursor`
  formatting against logical editor offsets.
- The context menu exposes `Plain` as a first-class formatting action. It routes through the same renderer path and
  clears all supported inline styles from the selected range before persisting canonical `.wsnbody`.
- Reapplying the same inline style to a fully formatted selection now clears that selection back to plain text instead
  of stacking duplicate RichText spans/source tags.
- Markdown presentation roles such as headings, blockquotes, link literals, or code literals must not by themselves make
  the controller think the corresponding shortcut style is already applied.
- The controller still persists canonical source tags (`<bold>`, `<italic>`, ...) directly; RichText HTML remains an
  editor-surface projection only.
- Formatting actions require a non-empty resolved selection range before mutating `.wsnbody`.

## Persistence Rules

- Mutations call `editorSession.markLocalEditorAuthority()` before persistence.
- Immediate persistence goes through `selectionBridge.persistEditorTextForNote(...)` when the contract is available.
- `persistEditorTextImmediately(...)` must return the real save result to its caller. The typing controller uses that
  boolean to decide whether debounce persistence should be re-armed.
- The save pipeline canonicalizes the edited RichText surface back into `.wsnbody` source tags, so shortcut/context
  formatting persists as semantic tags such as `<bold>...</bold>` and `<italic>...</italic>` instead of raw span CSS.
- Ordinary typing no longer goes through this controller or its whole-document RichText normalization helper.
- A successful immediate save now clears only the pending-save window through
  `ContentsEditorSession.acknowledgeSuccessfulEditorPersistence()`. It must not revoke local authority before the note
  list model echoes the same body text back, otherwise newly created notes can collapse the first typed text to a stale
  empty snapshot.
- If immediate persistence is unavailable or fails, the controller falls back to
  `editorSession.scheduleEditorPersistence()`.
- The host view still emits `editorTextEdited(...)`; the controller owns the mutation decision but not the broader
  editor-shell lifecycle.
- Hosts that expose `preferNativeInputHandling` can now override the editor surface policy:
  - desktop keeps the RichText editing surface
  - mobile/native-priority hosts force the live `TextEdit` surface back to `PlainText` so OS-native composition and
    selection heuristics are not re-hooked by RichText reconfiguration

## Regression Checks

- Re-selecting a different span and pressing `Cmd/Ctrl+B` / `I` / `U` / `Shift+X` / `Shift+E` should wrap the latest
  visible selection from the live `TextEdit`, not an older fallback snapshot.
- Pressing `Cmd+Shift+8` on macOS or `Alt+Shift+8` on Windows/Linux with a collapsed cursor on a plain line should
  insert/remove a canonical unordered list prefix for that line without needing a mouse selection first.
- Pressing `Cmd+Shift+7` on macOS or `Alt+Shift+7` on Windows/Linux with a collapsed cursor on a plain line should
  insert/remove a canonical ordered list prefix for that line without needing a mouse selection first.
- Applying either list shortcut to a multi-line selection should transform every touched non-empty line as one block,
  not only the first line.
- Reapplying the same list shortcut to a uniformly listed block should remove those markers instead of duplicating them.
- Converting between unordered and ordered shortcuts should rewrite the source prefixes canonically (`- ` or `N. `)
  rather than preserving mixed legacy markers inside one block.
- Formatting should apply to the exact rendered fragment under the current selection even when the note body already
  contains inline tags around nearby text.
- Applying the same shortcut twice to an already formatted selection should restore that selection to plain text.
- Choosing `Plain` from the context menu should remove all inline formatting tags from the selected source range.
- Shortcut and window-level accelerator paths should coalesce into one queued wrap request per note/tag pair, avoiding
  duplicate formatting from the same key chord.
- Heading/blockquotes/link/code markdown presentation must not cause `Bold` / `Italic` / `Underline` / `Highlight`
  toggles to misfire as if the proprietary `.wsnbody` style was already present.
- Immediate typing saves must return `true` on success so the typing controller does not schedule a redundant debounce
  write after the note was already persisted.
- A host with `preferNativeInputHandling` enabled must not be re-forced into `TextEdit.RichText` by the selection
  controller after note changes, surface syncs, or shortcut handling.
