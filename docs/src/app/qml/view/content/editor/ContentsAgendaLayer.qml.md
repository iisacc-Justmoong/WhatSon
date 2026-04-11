# `src/app/qml/view/content/editor/ContentsAgendaLayer.qml`

## Responsibility

`ContentsAgendaLayer.qml` renders proprietary `<agenda>` / `<task>` source blocks as an agenda card UI.

The layer is now a backend-fed view:
- input: canonical source text (`sourceText`)
- parser backend: `agendaBackend.parseAgendas(sourceText)` (`ContentsAgendaBackend`)
- output: LVRS card + `LV.CheckBox` task rows
- mutation hook: `taskToggleHandler(taskOpenTagStart, taskOpenTagEnd, checked)`

## Rendering Contract

- Root card style follows the Figma agenda frame direction:
  - card background `#262728`
  - stroke `#343536`
  - rounded corners (`LV.Theme.radiusMd`)
  - compact internal spacing/gaps
- Header row exposes:
  - left caption: `Agenda`
  - right date token: `agenda.date`
- Task rows are rendered as `LV.CheckBox`.
- `checked` is mapped from canonical `task.done` boolean.

## Parsing Rules

- Source parsing rules are implemented in `src/app/agenda/ContentsAgendaBackend.cpp`.
- This QML layer consumes only the backend parse result model and does not perform source-regex parsing locally.

## Mutation Hook

- On checkbox toggle, the layer calls the host callback with:
  - open-tag source start offset
  - open-tag source end offset
  - next checked state
- The host view owns actual source rewrite/persistence; this layer does not mutate source directly.

## Regression Checks

- A source `<agenda date="2026-04-11"><task done="false">A</task></agenda>` must render one agenda card with one unchecked checkbox.
- Toggling a checkbox must call `taskToggleHandler(...)` with stable open-tag offsets for that task.
- Multiple task rows in one agenda must preserve source order.
