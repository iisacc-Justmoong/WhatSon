# `src/app/qml/contents/Gutter.qml`

## Role
`Gutter.qml` renders the Figma `ContentsView` gutter rail for node `155:5345`.

## Contract
- Imports LVRS and uses LVRS color, metric, radius, and typography tokens.
- Exposes `lineNumberCount`, `activeLineNumber`, and marker color properties for the root contents frame.
- Emits `viewHookRequested(string reason)` through `requestViewHook(reason)` for the standard view hook surface.

## UI
- The token-composed host width is assigned by `ContentsView.qml`.
- The file owns the LVRS panel background, right-aligned line labels, and warning/success change markers.
