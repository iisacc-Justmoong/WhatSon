# `src/app/qml/view/panels/navigation/control/NavigationApplicationControlBar.qml`

## Role
`NavigationApplicationControlBar.qml` owns the application-level trailing control cluster for the shared navigation bar.

On desktop it mounts the full control row. On mobile compact mode it collapses into the right-edge menu trigger that sits beside the add-folder button.

## Compact Variants
The file exposes two compact variants:
- hierarchy/editor-control route: a `toolwindowtodo` menu trigger with the built-in LVRS chevron indicator, plus the
  optional right-edge `columnIndex` detail-page button when `compactDetailPanelVisible == true`
- note-list route: `sortByType`, `cwmPermissionView`, and the same `toolwindowtodo` menu trigger

The hierarchy/control compact trigger follows the Figma `174:4993` chrome rather than the old project-structure glyph. Both compact menu triggers use LVRS gap tokens for padding and `LV.Theme.gapNone` for the zero spacing case so the icon-plus-chevron footprint matches the mobile navigation bar frame.

## Menu Ownership
The file owns two context menus:
- `applicationControlContextMenu` for the hierarchy/control route
- `noteListApplicationControlContextMenu` for the compact note-list route

Both menus anchor from the trigger's bottom-right corner through `openFor(button, button.width, button.height + menuYOffset)`.

## Desktop Composition
The desktop row preserves the Figma child order:
1. `NavigationAppControlBar`
2. `NavigationExportBar`
3. `NavigationAddNewBar`
4. `NavigationPreferenceBar`

## Invariants
- Compact hierarchy mode must not regress to the `generalprojectStructure` menu glyph.
- Compact note-list mode must keep the order `sort -> visibility -> todo menu`.
- Menu items remain action-only entries with `keyVisible: false` and `showChevron: false`.

## Recent Updates
- Added `pragma ComponentBehavior: Bound` so compact/full mode nested `Component` branches can
  access `applicationControlBar` id members with LVRS-standard bound component scope.
- The mobile editor/control route now also renders the dedicated right-edge `columnIndex` detail button from
  Figma node `193:6606`, and that affordance now opens the dedicated mobile detail page directly instead of surfacing a
  duplicate context-menu action.
- That compact detail button now emits the hook reason `open-detail-page`, matching the routed-page interaction instead
  of the older collapse/expand overlay wording.
- Menu width/y-offset and the desktop full-row gap now route through `LV.Theme.inputMinWidth - LV.Theme.gap4` and
  `LV.Theme.gap2/12` instead of raw `176/2/12px` literals.
