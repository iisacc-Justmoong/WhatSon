# `src/app/qml/view/content/editor/ContentsAgendaLayer.qml`

## Responsibility

`ContentsAgendaLayer.qml` renders proprietary `<agenda>` / `<task>` source blocks as an agenda card UI.

The layer is now a renderer-fed view:
- input: renderer-owned agenda models (`renderedAgendas`)
- output: LVRS card + `LV.CheckBox` task rows
- mutation hook: `taskToggleHandler(taskOpenTagStart, taskOpenTagEnd, checked)`
- placement hook: `sourceOffsetYResolver(sourceStart)` (host-provided source-offset -> viewport-y resolver)

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

## Render-Model Rules

- Source parsing rules are now owned by `src/app/editor/renderer/ContentsStructuredBlockRenderer.cpp`.
- This QML layer consumes only renderer-provided agenda entries and does not parse RAW source locally.
- Parse entries include `sourceStart`; cards are positioned by source order/location, not by a fixed top-stacked
  column only.
- The layer still normalizes `QVariantList`-like values into JS arrays so C++ renderer properties remain QML-safe.

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
- Agenda cards must be placed at the resolved source location (`sourceOffsetYResolver`) so cards appear where tags are
  authored in note flow.
