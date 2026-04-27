# `src/app/qml/view/content/editor/ContentsAgendaBlock.qml`

## Responsibility
Renders one agenda card as a native document block inside the editor flow.

## Key Behavior
- Non-visual aggregate agenda behavior now lives in
  `src/app/models/editor/input/ContentsAgendaBlockController.qml`.
  Per-task typing state, toggle forwarding, cursor geometry, and committed task text emission live in
  `src/app/models/editor/input/ContentsAgendaTaskRowController.qml`.
- Fills the available editor width.
- Shows the agenda header/date frame using LVRS components and theme-scaled geometry.
- Renders each task as `LV.CheckBox` plus an inline plain-text editor.
- Emits three editor actions back to the flow host:
  - task text rewrite
  - `done` toggle rewrite
  - Enter handling for task continuation / agenda exit
- Restores task focus by matching either the reparsed task tag or the reparsed RAW content offset, so typing keeps the
  caret in the edited task instead of snapping back to the first row.
- Exposes `applyFocusRequest(...)` as a direct hook for `ContentsStructuredDocumentFlow.qml`; the block no longer
  watches global focus-request property churn on its own.
- The block now also exposes a derived focus state that stays true while any task editor or its checkbox owns focus.
  That lets the structured-flow host suppress idle snapshot/reparse work for the whole agenda card while one task is
  still actively being edited.
- The block now also exposes the currently focused task row as its local logical line number.
  Structured-flow hosts use that row index for current-line indicator placement so the blue gutter marker follows the
  focused agenda task rather than the card header or first task unconditionally.
- The agenda block now also exposes `currentCursorRowRect()` for the currently focused task editor.
  Structured-flow hosts use that row rectangle to align current-line gutter indicators with the actual visual caret row
  inside wrapped agenda task text.
- The block now also exposes `clearSelection(preserveFocusedEditor)` and clears every non-focused task editor on each
  structured activation turn.
  Agenda cards therefore cannot keep stale task selection highlight after focus moves to another task, block, or blank
  document area.
- The block now also exports the shared document-block contract for flow-level layout:
  `textEditable=true`, `atomicBlock=false`, `gutterCollapsed=false`,
  `minimapVisualKind=text`, `visiblePlainText()` as the joined task text, and
  `representativeCharCount(...)` per visible task line.
- Agenda task editors keep the last live task text emitted upward and pass it back as the expected previous field text.
  This prevents fast mobile typing/backspace from using a stale task snapshot as the next splice baseline.
- Agenda task rows no longer define custom boundary-key handlers.
  The nested `TextEdit` owns arrow navigation, Enter, iOS keyboard gestures, repeated delete, and selection behavior.
- Agenda task editors still receive the shared tag-management hook for clipboard-image resource paste. If the clipboard
  does not contain an importable image, the paste event is released back to native task text paste.
- Focus restoration now also accepts `entryBoundary: "before" | "after"` hints from the flow host.
  Sequential block traversal can therefore enter the agenda at its first task head or its last task tail instead of
  always collapsing back to the first task on reparsed focus restore.
- Keeps shortcut-created proprietary wrappers block-scoped by reporting the agenda boundary, not an in-task insertion
  point, for agenda/callout shortcut routing.
- Keeps empty tasks visible so RAW `<agenda>` tags always materialize into a card.
- The block now also accepts `paperPaletteEnabled`.
  Page/print mode therefore swaps the agenda chrome, task text, and checkbox colors away from the dark-theme
  white-on-charcoal palette into a light paper-safe palette.
