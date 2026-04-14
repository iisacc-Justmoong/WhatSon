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
- and routes the agenda insertion shortcut (`Cmd+Opt+T`) into the typing controller
- and routes the callout insertion shortcut (`Cmd+Opt+C`) into the typing controller
- and routes the divider insertion shortcut (`Cmd+Shift+H`) into the typing controller

## Public Contract

- `contextMenuSelectionStart` / `contextMenuSelectionEnd`: selection snapshot captured when the host primes the context
  menu selection on right-button press.
- `contextMenuItems`: menu model for inline formatting actions. The top item is `Plain`, which clears inline
  formatting from the current selection.
- `selectedEditorRange()`: resolves the current editor-surface selection span.
- `primeContextMenuSelectionSnapshot()`: captures the raw live `TextEdit` selection offsets and their plain-text slice
  before a right-click can collapse or shrink the editor selection.
- `openEditorSelectionContextMenu(localX, localY)`: opens the LVRS context menu when a non-empty selection exists.
- `wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange)`: applies the requested inline style to canonical
  `.wsnbody` source using the resolved logical editor selection range and persists the canonicalized result.
- `handleSelectionContextMenuEvent(eventName)`: routes menu events through the same formatting mutation path as
  keyboard shortcuts.
- `queueMarkdownListMutation(listKind)`: captures the current selection/cursor snapshot and toggles markdown list
  prefixes on the touched logical lines one event-loop turn later.
- `queueStructuredShortcutMutation(shortcutKind)`: queues agenda/callout/break insertion shortcut execution as one
  event-loop action to coalesce duplicate key-event/window-shortcut triggers.

## Range Mapping Rules

- The controller first prefers the live `TextEdit` selection snapshot exposed by `contentEditor.selectionSnapshot()`.
- When the wrapper exposes `currentPlainText()`, the controller now prefers that single helper for whole-surface
  plain-text inspection before falling back to direct `getText(...)` calls.
- Right-click context-menu flows now prime their selection snapshot on mouse press and reuse that cached range when the
  menu opens one event-loop turn later.
- The primed context-menu snapshot now prefers raw `selectionStart` / `selectionEnd` offsets and reconstructs the full
  selected plain text from `contentEditor.getText(...)`, so one stale/truncated `selectedText` fragment does not shrink
  a multi-block selection down to a single inline tag.
- Selection text normalization now also strips RichText object-replacement glyphs (`U+FFFC`) and normalizes NBSP back
  to ordinary spaces, so inline image/resource objects do not pollute plain-text selection inference.
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
- `Cmd+Opt+T` (`Meta+Alt+T`) now routes through `queueStructuredShortcutMutation("agenda")`, then into
  `view.queueAgendaShortcutInsertion()` when editor mutation guards allow it.
  - environments that surface Command as `ControlModifier` are also accepted through `Ctrl+Alt+T`.
- `Cmd+Opt+C` (`Meta+Alt+C`) now routes through `queueStructuredShortcutMutation("callout")`, then into
  `view.queueCalloutShortcutInsertion()` when editor mutation guards allow it.
  - environments that surface Command as `ControlModifier` are also accepted through `Ctrl+Alt+C`.
- `Cmd+Shift+H` (`Meta+Shift+H`) now routes through `queueStructuredShortcutMutation("break")`, then into
  `view.queueBreakShortcutInsertion()` when editor mutation guards allow it.
  - environments that surface Command as `ControlModifier` are also accepted through `Ctrl+Shift+H`.
- Agenda/callout shortcut key detection now accepts both `event.key` and single-character `event.text` fallback (`T` /
  `C` / `H`) so keyboard-layout-dependent keycode variance does not drop the shortcut.
- Agenda/callout/break shortcut key handling now also ignores shortcut auto-repeat events, preventing repeated raw-block
  insertion while the chord is held.
- Reapplying the same list shortcut to lines that are already uniformly in that list form removes the corresponding
  list markers instead of stacking duplicates.
- Mixed-line conversions are canonicalized:
  - unordered toggle converts ordered/plain lines into canonical `- ` items
  - ordered toggle converts unordered/plain lines into canonical `1. ` / `2. ` / `3. ` items
- Stored legacy/pasted bullet-glyph lines (`• item`) are treated as existing unordered-list lines for toggle/remove
  decisions and are canonicalized back to `- ` when the controller rewrites that block.
- Markdown list toggles now recompute rewritten line lengths through `ContentsLogicalTextBridge` instead of using raw
  source-text character counts. This keeps post-toggle selection spans and collapsed-cursor restoration aligned even
  when the touched line body contains inline tags, escaped entities, or `<resource ...>` tags that are wider in source
  than on the live logical editor surface.
- If one of those legacy inline-editor selection rewrites would reduce the total number of canonical
  `<resource ... />` tokens while the host is still outside structured-flow mode, the controller now aborts that
  rewrite and restores the surface from authoritative RAW presentation.
  Inline list/style commands therefore cannot silently strip body resource tags during a stale legacy-surface turn.
- The controller no longer mutates the live markdown-rendered RichText surface directly for shortcut formatting.
- It delegates to `ContentsTextFormatRenderer.applyInlineStyleToLogicalSelectionSource(...)`, which builds a
  markdown-neutral source-editing contract from the canonical `.wsnbody` text and now rebuilds proprietary RAW source
  tags directly from logical selection coverage instead of trusting `QTextDocument` fragment formatting as the
  authoritative boundary.
- Because ordinary typing now keeps `ContentsLogicalTextBridge` current through incremental adoption, explicit
  formatting/list actions no longer need a pre-mutation whole-document bridge commit.
- After a programmatic source rewrite finishes, the controller does trigger one immediate
  `commitDocumentPresentationRefresh()` so renderer/bridge/minimap state catches up to the rewritten note without
  waiting for the idle timer.
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
- Immediate editor mutations no longer bypass the fetch-sync boundary.
- `persistEditorTextImmediately(...)` now routes through `ContentsEditorSession.persistEditorTextImmediately(...)`,
  which forwards the current note snapshot into the buffered fetch-sync controller as one immediate flush request.
- That helper now returns the actual queue-acceptance result from
  `ContentsEditorSession.persistEditorTextImmediately(...)` instead of always reporting success.
- Callers can still inspect that boolean when they need queue-acceptance information, but the controller's own
  formatting/list mutations no longer downgrade rejected immediate writes back into deferred staging.
- The save pipeline canonicalizes the edited RichText surface back into `.wsnbody` source tags, so shortcut/context
  formatting persists as semantic tags such as `<bold>...</bold>` and `<italic>...</italic>` instead of raw span CSS.
- Ordinary typing no longer goes through this controller or its whole-document RichText normalization helper.
- Inline-format wraps and markdown-list rewrites now always request the immediate persistence path for the rewritten RAW
  source; the controller does not silently fall back to `editorSession.scheduleEditorPersistence()`.
- The host view still emits `editorTextEdited(...)`; the controller owns the mutation decision but not the broader
  editor-shell lifecycle.
- That immediate persistence path is now skipped when the candidate rewrite would drop one or more canonical
  `<resource ... />` tags from RAW while the host is still on the legacy inline-editor surface.
  In that failure case the controller restores the rendered editor surface from the last authoritative RAW projection
  instead of writing the damaged source.
- Hosts that expose `preferNativeInputHandling` can now override the editor surface policy:
  - desktop keeps the RichText editing surface
  - mobile/native-priority hosts force the live `TextEdit` surface back to `PlainText` so OS-native composition and
    selection heuristics are not re-hooked by RichText reconfiguration
- Whenever the controller has to restore a non-empty selection after rewriting source text, it now prefers
  `TextEdit.moveCursorSelection(...)` so the same active edge/cursor anchor survives the rewrite instead of being
  normalized by `select(start, end)`.

## Regression Checks

- Re-selecting a different span and pressing `Cmd/Ctrl+B` / `I` / `U` / `Shift+X` / `Shift+E` should wrap the latest
  visible selection from the live `TextEdit`, not an older fallback snapshot.
- Pressing `Cmd+Shift+8` on macOS or `Alt+Shift+8` on Windows/Linux with a collapsed cursor on a plain line should
  insert/remove a canonical unordered list prefix for that line without needing a mouse selection first.
- Pressing `Cmd+Shift+7` on macOS or `Alt+Shift+7` on Windows/Linux with a collapsed cursor on a plain line should
  insert/remove a canonical ordered list prefix for that line without needing a mouse selection first.
- Pressing `Cmd+Opt+T` while the editor is active should trigger exactly one agenda insertion request and must not
  double-insert when both key-event and window-shortcut paths are present.
- Pressing `Cmd+Opt+C` while the editor is active should trigger exactly one callout insertion request and must not
  double-insert when both key-event and window-shortcut paths are present.
- Pressing `Cmd+Shift+H` while the editor is active should trigger exactly one divider insertion request and must not
  double-insert when both key-event and window-shortcut paths are present.
- Holding the agenda/callout/break shortcut chord must not repeatedly append raw agenda/callout/divider blocks from
  key auto-repeat.
- In environments where Command is exposed as `ControlModifier`, `Ctrl+Alt+T` / `Ctrl+Alt+C` / `Ctrl+Shift+H` must
  trigger the same agenda/callout/divider insertion behavior as `Cmd+Opt+T` / `Cmd+Opt+C` / `Cmd+Shift+H`.
- Applying either list shortcut to a multi-line selection should transform every touched non-empty line as one block,
  not only the first line.
- Reapplying the same list shortcut to a uniformly listed block should remove those markers instead of duplicating them.
- Inline-format wraps and markdown-list toggles triggered while the editor has uncommitted live typing must still apply
  to the correct current logical selection; they should rely on the incrementally maintained bridge state instead of
  forcing a pre-mutation whole-document rebuild.
- Converting between unordered and ordered shortcuts should rewrite the source prefixes canonically (`- ` or `N. `)
  rather than preserving mixed legacy markers inside one block.
- Reapplying the unordered-list shortcut on a stored `• item` line should remove that bullet as an unordered-list marker
  instead of prepending a second canonical `- ` marker.
- Applying a markdown list toggle to a line whose body already contains inline tags or `<resource ...>` source tokens
  must keep the restored cursor and any rewritten selection anchored to the rendered logical text, not to the longer
  canonical source-token length.
- Selecting text near an inline RichText image/resource block must not include the editor's internal object glyph in
  inferred plain-text selection comparisons or context-menu range resolution.
- Reapplying a selection after inline/list mutations must preserve the active selection edge, so iOS keyboard-based
  range expansion or shrink does not collapse to only the newest traversed text fragment.
- Formatting should apply to the exact rendered fragment under the current selection even when the note body already
  contains inline tags around nearby text.
- Applying the same shortcut twice to an already formatted selection should restore that selection to plain text.
- Choosing `Plain` from the context menu should remove all inline formatting tags from the selected source range.
- Drag-selecting text across multiple rendered paragraphs or mixed inline tags, then right-clicking to format it, must
  keep the original whole selection instead of reformatting only the fragment nearest the click.
- Shortcut and window-level accelerator paths should coalesce into one queued wrap request per note/tag pair, avoiding
  duplicate formatting from the same key chord.
- Heading/blockquotes/link/code markdown presentation must not cause `Bold` / `Italic` / `Underline` / `Highlight`
  toggles to misfire as if the proprietary `.wsnbody` style was already present.
- Applying `Strikethrough` through shortcut/context-menu on indented markdown lines must keep line indentation and
  structural list/heading prefixes intact after the source rewrite.
- Immediate typing saves must return `true` once the snapshot is accepted into the buffered fetch-sync flush path so the
  typing controller does not schedule a redundant second staging request on the same turn.
- Immediate persistence helpers must not report success when the staging request was rejected, because selection or
  correction flows may otherwise continue as if the current note snapshot were safe.
- Immediate persistence helpers must not silently downgrade to deferred-only staging, because the editor host now
  treats formatting/list rewrites as write-through `.wsnbody` updates by default.
- A host with `preferNativeInputHandling` enabled must not be re-forced into `TextEdit.RichText` by the selection
  controller after note changes, surface syncs, or shortcut handling.
