# `src/app/qml/view/panels/navigation/view/NavigationApplicationViewBar.qml`

## Responsibility
`NavigationApplicationViewBar.qml` renders the right-side application toolbar cluster when the
navigation mode is `View`.

The Figma node mapping is:
- `149:4000` `ApplicationViewBar`
- child order: `CalendarBar -> AddNewBar -> PreferenceBar`

## Layout Contract
- Root object: `Item`
- Public mode switch: `property bool compactMode`
- Full mode (`compactMode: false`):
  - `NavigationApplicationCalendarBar`
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
- Full-mode `NavigationApplicationCalendarBar` hooks are now forwarded back into
  `requestViewHook(reason)`, so year-calendar clicks in icon mode and compact-menu mode share the
  same reason pipeline.
- Compact menu mirrors full-mode default tools:
  - Agenda / Daily / Weekly / Monthly / Yearly calendar entries
  - New File
  - Preferences

## Panel ViewModel Binding
- Panel key: `navigation.NavigationApplicationViewBar`
- Binding: `panelViewModelRegistry.panelViewModel("navigation.NavigationApplicationViewBar")`

## Notes
- Full-mode child frames reuse shared wrappers from `navigation/` so view and edit modes avoid
  duplicated mode-local wrapper files.
- On mobile compact shell, this menu button is rendered alongside (not instead of) the shared
  `nodesnewFolder` add-folder button from `NavigationBarLayout.qml`.
- Compact trigger icon now matches control mode (`toolwindowtodo`) so mobile mode bars keep one
  consistent menu affordance pattern.
- Compact trigger follows the shared menu-button padding contract used by control mode:
  `left=2`, `right=4`, `top=2`, `bottom=2`, `spacing=0`.
- The compact editor route now also renders the dedicated right-edge `columnIndex` detail button from
  Figma node `193:6606`, with the same `toggleDetailPanelRequested` signal path used by the desktop preference bar.
- That compact detail affordance opens the dedicated mobile detail page directly; it is intentionally not duplicated as
  a context-menu entry.
- The compact detail button now emits the hook reason `open-detail-page`, matching the routed-page behavior instead of
  the older collapse/expand overlay wording.
- `pragma ComponentBehavior: Bound` is enabled so compact/full nested `Component` branches can
  access `applicationViewBar` id members without unqualified-scope warnings.
