# `src/app/qml/view/content/editor/ContentsAgendaLayer.qml`

## Current Behavior
- The layer now also accepts `paperPaletteEnabled`.
  Even when it is only being used as an editor-side visual overlay, page/print mode no longer leaves hardcoded white
  task/header text colors in this projection path.

## Responsibility

`ContentsAgendaLayer.qml` renders proprietary `<agenda>` / `<task>` source blocks as an agenda card UI.

The layer is now a renderer-fed view:
- input: renderer-owned agenda models (`renderedAgendas`)
- output: LVRS card + `LV.CheckBox` task rows
- focus hook: `blockFocusHandler(focusSourceOffset)`
- mutation hook: `taskToggleHandler(taskOpenTagStart, taskOpenTagEnd, checked)`
- placement hook: `sourceOffsetYResolver(sourceStart)` (host-provided source-offset -> viewport-y resolver)
- host mode flags:
  - `showFrame` / `showHeader`
  - `showTaskCheckbox` / `showTaskText`
  - `enableTaskToggle` / `enableCardFocus`

## Rendering Contract

- Root card style follows the Figma agenda frame direction:
  - card background `#262728`
  - stroke `#343536`
  - corner radius `12`
  - outer padding `8`
  - card width now stretches across the resolved editor text column so agenda cards fill the document X-axis
- Internal Figma spacing is preserved:
  - header -> task list gap `8`
  - task list horizontal inset `8`
  - task row gap `2`
  - checkbox -> text gap `6`
- Header row exposes:
  - left caption: `Agenda`
  - right date token: `agenda.date`
- Task rows now split visual responsibilities:
  - `LV.CheckBox` still owns the checkbox box/toggle affordance
  - a sibling `LV.Label` owns the measurable/wrapping task text
  - this lets the host reuse the same component for a background-only chrome pass and a checkbox-only interaction pass
- `checked` is mapped from canonical `task.done` boolean.
- Checkbox visuals are pinned to the Figma card:
  - box size `17`
  - box radius `3.5`
  - white body fill / border
  - dark check mark drawn against the card background

## Render-Model Rules

- Source parsing rules are now owned by `src/app/editor/renderer/ContentsStructuredBlockRenderer.cpp`.
- This QML layer consumes only renderer-provided agenda entries and does not parse RAW source locally.
- Parse entries include `sourceStart` and `focusSourceOffset`; cards are positioned by source order/location and can
  restore editor focus to the first task body.
- Render entries may now be provisional:
  - open-only `<agenda ...>` blocks still arrive here
  - task rows may have `tagVerified=false`
  - placeholder rows may have `hasSourceTag=false` when the agenda exists but no `<task>` tag has been authored yet
- The host-provided `sourceOffsetYResolver(...)` now resolves editor-internal document Y, so card positioning stays in
  the same coordinate space as the live editor content instead of double-counting outer viewport margins.
- The layer still normalizes `QVariantList`-like values into JS arrays so C++ renderer properties remain QML-safe.
- The host can now mount this layer twice:
  - one frame/header pass below the live editor text
  - one checkbox-only interaction pass above the live editor text
  This keeps agenda chrome document-integrated while preserving task toggle affordances.

## Mutation Hook

- On checkbox toggle, the layer calls the host callback with:
  - open-tag source start offset
  - open-tag source end offset
  - next checked state
- Checkbox toggles now guard against model-echo transitions (`checked === task.done`) before calling the host callback,
  so renderer refreshes do not re-trigger the same `done` mutation loop.
- On card tap, the layer calls `blockFocusHandler(focusSourceOffset)` so empty agenda cards can immediately route the
  cursor back into RAW task scope.
- The host view owns actual source rewrite/persistence; this layer does not mutate source directly.
- Root layer and each agenda card now bind `height: implicitHeight`, so renderer-fed agenda entries no longer collapse
  into zero-height rectangles when the overlay is mounted in the editor viewport.
- When task text is visually hidden (`showTaskText=false`), the label still stays in layout so agenda chrome can keep
  measuring multi-line row height against the same renderer-fed task body text.

## Regression Checks

- A source `<agenda date="2026-04-11"><task done="false">A</task></agenda>` must render one agenda card with one unchecked checkbox.
- A source `<agenda date="2026-04-11">` must still render one visible agenda card with one blank task row.
- A source `<agenda date="2026-04-11"><task done="false">A` must still render one visible agenda card while parser
  verification remains non-well-formed.
- Toggling a checkbox must call `taskToggleHandler(...)` with stable open-tag offsets for that task.
- Multiple task rows in one agenda must preserve source order.
- Agenda cards must be placed at the resolved source location (`sourceOffsetYResolver`) so cards appear where tags are
  authored in note flow.
- Tapping an agenda card must route focus back to the first task body source offset.
- Even when the editor overlay root has no explicit bottom anchor, agenda cards must still reserve real frame height
  and remain visible.
- A fill-width host mount must stretch agenda cards across the editor text column instead of capping card width to the
  earlier fixed `307px` frame preference.
