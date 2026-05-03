# `src/app/qml/contents/ContentsView.qml`

## Role
`ContentsView.qml` implements Figma node `155:4561` as a standalone contents editor frame.

## Composition
- Root `ContentsView`: LVRS dark panel token, token-composed Figma frame size, shared hook signal.
- Child `LV.HStack`: Figma node `155:5344`, ordered left-to-right as `Gutter.qml`, `EditorView.qml`, then `Minimap.qml`.
- `Gutter.qml`: token-composed rail with line numbers, active line color, and change/conflict markers.
- `EditorView.qml`: read-only text projection using `LV.TextEditor`, LVRS body typography, and native selection.
- `Minimap.qml`: token-composed rail made from repeated LVRS hairline rows.

## Contract
- UI artifact count stays at four source files: `ContentsView.qml`, `Gutter.qml`, `EditorView.qml`, and `Minimap.qml`.
- `ContentsView.qml` embeds those sibling parts through normal QML composition and owns the shared frame properties.
- The `LV.HStack` child order is part of the Figma contract and must remain `Gutter -> EditorView -> Minimap`.
- Hardcoded color, spacing, and typography literals are prohibited; use `LV.Theme` tokens or token compositions.
- Regression coverage rejects direct numeric assignments for contents-view metric properties.
- The editor projection is `readOnly: true`; this view must not become a write-authoritative `.wsnbody` surface.
- Hook entrypoint: `requestViewHook(reason)`.
- Output signal: `viewHookRequested(string reason)`.

## Verification
- `qmlContentsView_composesFigmaFrameFromLvrsParts`
- `qmlContentsView_partsKeepEditorProjectionReadOnlyAndNativeInputSafe`
