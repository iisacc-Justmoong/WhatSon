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
- `itemWidth` is `141`, matching the Figma context-menu frame width so icon + label rows do not compress.
- `selectedIndex` must resolve from `editorViewModeViewModel.activeViewMode`, so the currently active editor view
  remains highlighted when the menu opens.

## Interaction Contract

- Clicking the combo box toggles the menu through `toggleEditorViewMenu()`.
- Choosing a menu entry must call `editorViewModeViewModel.requestViewModeChange(index)`.
- The view still emits `viewHookRequested` after menu open/select so panel-level hook instrumentation remains intact.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist for this file:
  - The menu entry labels remain exactly `Plain`, `Page`, `Print`, `Web`, `Presentation`.
  - Each menu entry keeps the Figma-aligned `iconName` token listed above.
  - Opening the menu while each editor view is active highlights the matching row through `selectedIndex`.
  - The `Presentation` row fits without clipping inside the `141px` menu width.
