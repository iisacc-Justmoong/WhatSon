# `src/app/qml/view/content/editor/DrawerMenuBar.qml`

## Responsibility

`DrawerMenuBar.qml` renders the top frame of the lower editor drawer. It mirrors Figma frame `155:4565`
(`DrawerMenubar`) and keeps mode switching separate from drawer body rendering.

## Frame Contract

- Root frame identity is preserved through `objectName: "DrawerMenubar"` and `figmaNodeId: "155:4565"`.
- Root id is `DrawerMenubar`, and mode/config actions route through id-scoped calls (`DrawerMenubar.*`) for stable
  delegate scope binding.
- The left segment group is `DrawerModes` (`155:4566`) and keeps four stable child ids:
  `QuickNote`, `ItemBox`, `DataSearch`, and `GraphView`.
- The right control group is `DrawerViewConfig` (`155:4567`) and keeps `TextAlign`, `ViewOptions`, and
  `editorPreviewVertical`.

## Visual Rules

- The bar background stays on `LV.Theme.panelBackground02`.
- `DrawerModes` keeps the Figma segmented shell tokens: `panelBackground08` fill, `panelBackground12` border, `8px`
  radius, `4px` outer padding, and `2px` segment spacing.
- Only the active drawer mode uses `LV.AbstractButton.Default`; inactive modes stay borderless.

## Behavior

- `drawerModeRequested(modeName)` emits intent only. The parent surface decides whether a requested mode is currently
  implemented.
- `drawerConfigActionRequested(actionName)` exposes the right-side controls without coupling this bar to persistence or
  editor logic.
- `requestViewHook(reason)` is the local hook entrypoint for panel-level wiring.
