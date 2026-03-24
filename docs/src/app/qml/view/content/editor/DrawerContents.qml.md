# `src/app/qml/view/content/editor/DrawerContents.qml`

## Responsibility

`DrawerContents.qml` renders the middle body of the lower editor drawer. In the current scope it implements only the
Quick Note mode from Figma frame `174:6352`.

## Frame Contract

- Root frame identity is preserved through `objectName: "DrawerContents"` and `figmaNodeId: "174:6352"`.
- The rendered page surface is `QuickNotePage` (`174:6350`), centered horizontally and capped at `511px` width.
- The drawer content uses `16px` horizontal padding and `8px` vertical padding around the page surface.

## Quick Note Page

- The page is inline QML content, not a `Window`.
- Editing is provided by `LV.TextEditor`, which keeps the drawer aligned with the LVRS input contract.
- The editor background is fully transparent because the drawer body should sit directly on the editor canvas.
- A `Binding` forces the embedded LVRS text item `topPadding` and `bottomPadding` to `0`; otherwise the default LVRS
  vertical-centering behavior would float the text block away from the Figma top edge.

## Public Surface

- `quickNoteText` is the mutable page text alias.
- `quickNoteTextEdited(text)` reports live edits.
- `quickNoteSubmitted(text)` exposes the platform submit gesture.
- `requestViewHook(reason)` remains the local hook entrypoint for any later integration with drawer-specific viewmodels.
