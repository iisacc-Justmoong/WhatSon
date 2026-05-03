# `src/app/qml/view/contents/ContentsView.qml`

## Role
`ContentsView.qml` implements Figma node `155:4561` as a standalone contents editor frame.

## Composition
- Root `ContentsView`: LVRS dark panel token, token-composed Figma frame size, shared hook signal.
- Child `LV.HStack`: Figma node `155:5344`, ordered left-to-right as `Gutter.qml`, `EditorView.qml`, then `Minimap.qml`.
- `Gutter.qml`: transparent rail with line numbers, active line color, and semantic cursor/unsaved markers.
- `EditorView.qml`: read-only text projection using `LV.TextEditor`, LVRS body typography, and native selection.
- `Minimap.qml`: token-composed rail made from repeated LVRS hairline rows.
- `ContentsGutterLayoutMetrics` and `ContentsMinimapLayoutMetrics`: C++ metric calculators imported from
  `WhatSon.App.Internal` so this Figma frame does not own gutter/minimap arithmetic.
- `ContentsGutterLineNumberGeometry`: C++ line-number entry projector used by the gutter, with fallback y positions
  for this standalone frame when no live editor geometry host is mounted. The fallback starts at zero top inset to
  match the top-flush editor text projection.
- `ContentsGutterMarkerGeometry`: C++ marker projector used by the gutter; the standalone design frame leaves the
  editor unmounted, while the runtime editor supplies live cursor and saved-source inputs.

## Contract
- UI artifact count stays at four source files: `ContentsView.qml`, `Gutter.qml`, `EditorView.qml`, and `Minimap.qml`.
- `ContentsView.qml` embeds those sibling parts through normal QML composition and owns the shared frame properties.
- The `LV.HStack` child order is part of the Figma contract and must remain `Gutter -> EditorView -> Minimap`.
- Hardcoded color, spacing, and typography literals are prohibited; use `LV.Theme` tokens or token compositions.
- Gutter and minimap sizing/count calculations must be consumed from `src/app/models/editor/gutter` and
  `src/app/models/editor/minimap`.
- Gutter line-number y positions must be consumed through `ContentsGutterLineNumberGeometry`; `Gutter.qml` must not
  stack line labels independently.
- Gutter marker y positions must be consumed through `ContentsGutterMarkerGeometry`; `Gutter.qml` must not infer cursor
  or unsaved state from presentation text.
- Regression coverage rejects direct numeric assignments for contents-view metric properties.
- The editor projection is `readOnly: true`; this view must not become a write-authoritative `.wsnbody` surface.
- Hook entrypoint: `requestViewHook(reason)`.
- Output signal: `viewHookRequested(string reason)`.

## Verification
- `qmlContentsView_composesFigmaFrameFromLvrsParts`
- `qmlContentsView_partsKeepEditorProjectionReadOnlyAndNativeInputSafe`
