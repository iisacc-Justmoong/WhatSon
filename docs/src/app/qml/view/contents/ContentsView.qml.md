# `src/app/qml/view/contents/ContentsView.qml`

## Role
`ContentsView.qml` implements Figma node `155:4561` as a standalone contents editor frame.

## Composition
- Root `ContentsView`: LVRS dark panel token, token-composed Figma frame size, shared hook signal.
- Child `LV.HStack`: Figma node `155:5344`, ordered left-to-right as `EditorView.qml`, then `Minimap.qml`.
- `EditorView.qml`: read-only text projection using `LV.TextEditor`, LVRS body typography, and native selection.
- `Minimap.qml`: token-composed rail made from repeated LVRS hairline rows.
- `ContentsMinimapLayoutMetrics`: C++ metric calculator imported from `WhatSon.App.Internal` so this Figma frame does
  not own minimap arithmetic.

## Contract
- UI artifact count for this frame is now three source files: `ContentsView.qml`, `EditorView.qml`, and `Minimap.qml`.
- `ContentsView.qml` embeds those sibling parts through normal QML composition and owns the shared frame properties.
- The `LV.HStack` child order is part of the current contract and must remain `EditorView -> Minimap`.
- Hardcoded color, spacing, and typography literals are prohibited; use `LV.Theme` tokens or token compositions.
- Minimap sizing/count calculations must be consumed from `src/app/models/editor/minimap`.
- Regression coverage rejects direct numeric assignments for contents-view metric properties.
- The editor projection is `readOnly: true`; this view must not become a write-authoritative `.wsnbody` surface.
- Hook entrypoint: `requestViewHook(reason)`.
- Output signal: `viewHookRequested(string reason)`.

## Verification
- `qmlContentsView_composesFigmaFrameFromLvrsParts`
- `qmlContentsView_partsKeepEditorProjectionReadOnlyAndNativeInputSafe`
