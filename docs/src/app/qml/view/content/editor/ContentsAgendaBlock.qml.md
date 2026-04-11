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
- Keeps empty tasks visible so RAW `<agenda>` tags always materialize into a card.
