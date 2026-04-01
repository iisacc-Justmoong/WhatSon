# `src/app/qml/view/panels/navigation/edit/NavigationApplicationEditBar.qml`

## Responsibility
`NavigationApplicationEditBar.qml` renders the right-side application toolbar cluster when the
navigation mode is `Edit`.

The Figma node mapping is:
- `149:4102` `ApplicationEditBar`
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
  - Show/Hide Detail Panel

## Panel ViewModel Binding
- Panel key: `navigation.NavigationApplicationEditBar`
- Binding: `panelViewModelRegistry.panelViewModel("navigation.NavigationApplicationEditBar")`

## Notes
- Full-mode child frames reuse shared wrappers from `navigation/` so view and edit modes avoid
  duplicated mode-local wrapper files.
- On mobile compact shell, this menu button is rendered alongside (not instead of) the shared
  `nodesnewFolder` add-folder button from `NavigationBarLayout.qml`.
- Compact trigger icon now matches control mode (`toolwindowtodo`) so mobile mode bars keep one
  consistent menu affordance pattern.
- Compact trigger follows the shared menu-button padding contract used by control mode:
  `left=2`, `right=4`, `top=2`, `bottom=2`, `spacing=0`.
- The detail-panel menu label is dynamic:
  `"Show Detail Panel"` when collapsed, `"Hide Detail Panel"` otherwise.
- `pragma ComponentBehavior: Bound` is enabled so compact/full nested `Component` branches can
  access `applicationEditBar` id members without unqualified-scope warnings.
