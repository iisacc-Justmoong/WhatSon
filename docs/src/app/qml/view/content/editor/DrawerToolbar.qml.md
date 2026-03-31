# `src/app/qml/view/content/editor/DrawerToolbar.qml`

## Responsibility

`DrawerToolbar.qml` renders the bottom action bar of the lower editor drawer. It mirrors Figma frame `155:4570` and
keeps action triggers separate from the drawer body.

## Frame Contract

- Root frame identity is preserved through `objectName: "DrawerToolbar"` and `figmaNodeId: "155:4570"`.
- Root id is `drawerToolbar`, and action triggers dispatch through `drawerToolbar.*` to keep delegate scope explicit.
- The right-aligned action group id is `sumitGroup`; the objectName remains `Sumit` because that is the exact Figma frame label
  (`155:4571`).
- `showQuickNoteWindowButton` (`155:4572`) is the pop-out action for the Quick Note view.
- `newDraftButton` (`155:4573`) is the filled `New Draft` action button.

## Visual Rules

- The toolbar background stays on `LV.Theme.panelBackground02`.
- The pop-out action remains borderless with `2px` padding around the `jpaConsoleToolWindow` icon.
- `NewDraft` uses the default filled LVRS button tone with `8px` horizontal and `4px` vertical padding to match the
  Figma filled pill.

## Behavior

- `showQuickNoteWindowRequested()` and `newDraftRequested()` emit intent only.
- `requestViewHook(reason)` remains the local hook entrypoint so the parent editor surface can forward actions to the
  current panel-level owner without embedding drawer business logic in this file.
