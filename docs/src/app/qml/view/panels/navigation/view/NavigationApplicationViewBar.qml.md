# `src/app/qml/view/panels/navigation/view/NavigationApplicationViewBar.qml`

## Responsibility
`NavigationApplicationViewBar.qml` renders the right-side application toolbar cluster when the
navigation mode is `View`.

The Figma node mapping is:
- `149:4000` `ApplicationViewBar`
- child order: `ViewOptionBar -> ModeBar -> CalendarBar -> AddNewBar -> PreferenceBar`

## Layout Contract
- Root object: `Item`
- Public mode switch: `property bool compactMode`
- Full mode (`compactMode: false`):
  - `NavigationApplicationViewOptionBar`
  - `NavigationApplicationViewModeBar`
  - `NavigationApplicationViewCalendarBar`
  - `NavigationApplicationAddNewBar`
  - `NavigationApplicationPreferenceBar`
- Compact mode (`compactMode: true`):
  - one `LV.IconMenuButton` trigger
  - one optional `LV.IconButton` detail-panel trigger when `compactDetailPanelVisible == true`
  - one `LV.ContextMenu` rendered on `Controls.Overlay.overlay`

## Interaction Contract
- Exposes `toggleDetailPanelRequested` and forwards it from `NavigationPreferenceBar`.
- Exposes `viewHookRequested` and forwards mode-level hook reasons through
  `panelViewModel.requestViewModelHook(reason)`.
- Full-mode view-only child bars (`ViewOptionBar`, `ModeBar`, `CalendarBar`) forward their hooks back
  into `requestViewHook(reason)`, so full-mode icons and compact-menu rows share one reason pipeline.
- Compact menu mirrors full-mode default tools:
  - View options (`Read Only`, `Wrap Text`, `Center View`, `Text To Speech`, `Paper Options`)
  - View modes (`Center View Mode`, `Focus Mode`, `Presentation`)
  - Agenda / Daily / Weekly / Monthly / Yearly calendar entries
  - New File
  - Preferences

## Panel ViewModel Binding
- Panel key: `navigation.NavigationApplicationViewBar`
- Binding: `panelViewModelRegistry.panelViewModel("navigation.NavigationApplicationViewBar")`

## Notes
- Full-mode child frames now split the view-only Figma slices into dedicated local files under
  `navigation/view/`, while `AddNewBar` and `PreferenceBar` stay shared under `navigation/`.
- The view-only calendar cluster no longer reuses the shared root `NavigationCalendarBar.qml`, because
  Figma `149:4001` changed the first icon from `toolWindowCheckDetails` to `validator` only for the
  view-mode `ApplicationViewBar`.
- On mobile compact shell, this menu button is rendered alongside (not instead of) the shared
  `nodesnewFolder` add-folder button from `NavigationBarLayout.qml`.
- Compact trigger icon now matches control mode (`toolwindowtodo`) so mobile mode bars keep one
  consistent menu affordance pattern.
- Compact trigger follows the shared menu-button padding contract used by control mode:
  `left=2`, `right=4`, `top=2`, `bottom=2`, `spacing=0`.
- Compact menu width/y-offset now route through `LV.Theme.scaleMetric(196)` and `LV.Theme.gap2`.
- The `Center View` option keeps the Figma `258:8039` target/reticle glyph via LVRS `recursiveMethod`,
  while `Center View Mode` keeps the separate LVRS `singleRecordView` screen-preview glyph from `258:7852`.
- The compact editor route now also renders the dedicated right-edge `columnIndex` detail button from
  the `DetailPanelControlButton` affordance, exposed locally as `detailPanelControlButton`, with the same
  `toggleDetailPanelRequested` signal path used by the desktop preference bar.
- That compact detail affordance opens the dedicated mobile detail page directly; it is intentionally not duplicated as
  a context-menu entry.
- The compact detail button now emits the hook reason `open-detail-page`, matching the routed-page behavior instead of
  the older collapse/expand overlay wording.
- `pragma ComponentBehavior: Bound` is enabled so compact/full nested `Component` branches can
  access `applicationViewBar` id members without unqualified-scope warnings.

## Regression Checklist
- Keep the child-frame order `ViewOptionBar -> ModeBar -> CalendarBar -> AddNewBar -> PreferenceBar`.
- Keep the first calendar compact-menu entry on `validator`, matching Figma `TodoListButton`.
- Keep the `Center View` option icon on `recursiveMethod`; do not collapse it back onto the mode-level `singleRecordView` glyph.
- Keep the compact-menu section ordering aligned with the full desktop bar ordering.
