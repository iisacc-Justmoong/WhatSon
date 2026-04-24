# `src/app/qml/view/content/editor/ContentsEditorSelectionController.qml`

## Responsibility

`ContentsEditorSelectionController.qml` owns selection resolution, inline style tag formatting, and selection
context-menu dispatch for `ContentsDisplayView.qml`.

It does not own ordinary typing, markdown list input, or generic key-event shortcut parsing.

## Public Contract

- `contextMenuSelectionStart` / `contextMenuSelectionEnd`: selection snapshot captured before a right-click can collapse
  or shrink the live `TextEdit` selection.
- `contextMenuItems`: menu model for inline tag formatting actions. The top item is `Plain`, which clears supported
  inline style tags from the current selection.
- `selectedEditorRange()`: resolves the current live editor selection span.
- `primeContextMenuSelectionSnapshot()`: captures raw `TextEdit` selection offsets and selected plain text.
- `openEditorSelectionContextMenu(localX, localY)`: opens the LVRS context menu when a non-empty selection exists.
- `wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange)`: applies or clears the requested inline style tag in
  canonical `.wsnbody` source.
- `handleSelectionContextMenuEvent(eventName)`: routes context-menu events through the same source-tag mutation path.
- `queueInlineFormatWrap(tagName)`: executes an explicit tag-management command from the host shortcut surface.

## Input Policy

- This controller is a tag-management collaborator only.
- It must not expose `handleInlineFormatShortcutKeyPress(...)`, markdown-list shortcut queues, structured shortcut
  queues, or any QML key-event parser.
- Markdown list toggles are intentionally not provided from the editor input layer.
- Ordinary `Enter`, `Backspace`, `Delete`, arrow navigation, selection extension, repeat, and IME gestures must remain
  native `TextEdit` behavior.

## Mutation Rules

- Inline style actions require a non-empty resolved selection range before mutating `.wsnbody`.
- The controller first prefers `contentEditor.inlineFormatSelectionSnapshot()` so a shortcut turn can reuse the last
  non-empty selection even if Qt briefly collapses the live range.
- Right-click context-menu flows prime their selection snapshot on mouse press and reuse that cached range when the menu
  opens one event-loop turn later.
- Selection text normalization strips RichText object-replacement glyphs (`U+FFFC`) and normalizes NBSP back to ordinary
  spaces.
- The controller delegates source rewriting to `ContentsTextFormatRenderer.applyInlineStyleToLogicalSelectionSource(...)`
  and persists semantic tags such as `<bold>...</bold>` and `<italic>...</italic>`.
- Before the legacy whole-editor path runs, the controller asks the structured flow whether the active block can handle
  the inline-format tag command locally.
- If a candidate rewrite would reduce the number of canonical `<resource ... />` tokens while the host is on the legacy
  inline-editor surface, the controller aborts and restores the surface from authoritative RAW presentation.

## Persistence Rules

- Accepted tag-management mutations mark local editor authority before persistence.
- Programmatic source rewrites trigger one immediate presentation refresh so renderer, bridge, gutter, and minimap state
  catch up without waiting for the idle timer.
- Ordinary typing never goes through this controller.

## Regression Checks

- Inline format commands must operate on the current live selection, including structured paragraph selections.
- Reapplying the same inline style to a fully formatted selection should clear that style.
- Choosing `Plain` from the context menu should remove all supported inline style tags from the selected source range.
- Drag-selected multi-paragraph text should keep the original selection when the context menu applies a tag.
- The controller source must not contain markdown-list queues, structured shortcut queues, or generic key-event shortcut
  handlers.
