# `src/app/qml/view/content/editor/ContentsAgendaBlock.qml`

## Responsibility
Renders one agenda card as a native document block inside the editor flow.

## Key Behavior
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
- Keeps shortcut-created proprietary wrappers block-scoped by reporting the agenda boundary, not an in-task insertion
  point, for agenda/callout shortcut routing.
- Keeps empty tasks visible so RAW `<agenda>` tags always materialize into a card.
