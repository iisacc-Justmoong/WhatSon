# `src/app/qml/view/panels/navigation/NavigationEditorViewBar.qml`

## Responsibility

`NavigationEditorViewBar.qml` owns the editor `View mode` selector mounted in the navigation strip.
It shows the currently active editor view label in an `LV.ComboBox`, opens an `LV.ContextMenu`, and
routes the selected index back into `EditorViewModeViewModel.requestViewModeChange(...)`.

## Source Metadata

- Source path: `src/app/qml/view/panels/navigation/NavigationEditorViewBar.qml`
- Source kind: QML view/component
- Root type: `LV.HStack`
- Key ids:
  - `editorViewBar`
  - `editorViewCombo`
  - `editorViewContextMenu`

## Surface Contract

- `editorViewMenuItems` is the single source of truth for the context-menu entry order, labels, icons, and selected
  state.
- `showLabel` controls whether the left `View` label is rendered. Mobile compact navigation sets
  `showLabel: false` so the selector can fit inside the `174:5689` dual-combo group.
- The menu keeps the editor-view enum order from `EditorViewState`:
  - `0`: `Plain`
  - `1`: `Page`
  - `2`: `Print`
  - `3`: `Web`
  - `4`: `Presentation`
- The context menu now matches Figma node `192:8693` for the important visible fields:
  - `Plain` -> `string`
  - `Page` -> `fileSet`
  - `Print` -> `generalprint`
  - `Web` -> `toolwindowweb`
  - `Presentation` -> `procedure`
- `itemWidth` is routed through a named `LV.Theme` token composition matching the Figma context-menu frame width while
  still scaling with LVRS UI density.
- When `showLabel: false`, the combo width is constrained through a named `LV.Theme` token composition so it matches
  the compact mobile navigation slot width from Figma node `174:6000`.
- `selectedIndex` must resolve from `editorViewModeViewModel.activeViewMode`, so the currently active editor view
  remains highlighted when the menu opens.

## Interaction Contract

- Clicking the combo box toggles the menu through `toggleEditorViewMenu()`.
- The popup vertical offset is routed through `comboMenuYOffset` (`LV.Theme.gap2`) instead of a fixed literal.
- Choosing a menu entry must call `editorViewModeViewModel.requestViewModeChange(index)`.
- The view still emits `viewHookRequested` after menu open/select so panel-level hook instrumentation remains intact.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist for this file:
  - The menu entry labels remain exactly `Plain`, `Page`, `Print`, `Web`, `Presentation`.
  - Each menu entry keeps the Figma-aligned `iconName` token listed above.
  - Opening the menu while each editor view is active highlights the matching row through `selectedIndex`.
  - The `Presentation` row fits without clipping inside the token-composed menu width.
  - Mobile compact navigation (`showLabel: false`) keeps the combo width at the token-composed compact slot and still opens the same context menu.
