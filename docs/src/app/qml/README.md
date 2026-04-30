# `src/app/qml`

## Role
This directory contains the root LVRS application shell and the top-level QML-side interaction controllers.

The core rule in this directory is that visual composition belongs here, while persistence and most mutation logic should stay in C++ viewmodels or in narrow bridge/helper objects.

## Important Files
- `Main.qml`: the root `LV.ApplicationWindow`, route shell, and root `LV.ViewModels` ownership point.
- `MainWindowInteractionController.qml`: shortcut, resize-policy, and focus-management helper bound to the root window.
- `DesignTokens.qml`: QML-side design token aggregation.
- `contents/ContentsView.qml`: standalone Figma `ContentsView` frame that embeds the sibling gutter, editor, and
  minimap QML parts using `LV.Theme` tokens for colors, spacing, typography, and fixed rails.

## LVRS Token Rule
- QML view files must express reusable colors, transparency, spacing, typography, and fixed UI extents through
  `LV.Theme` tokens or token compositions.
- Direct visual literals such as `#...`, `Qt.rgba(...)`, `color: "transparent"`, `spacing: 0`, and raw
  `font.pixelSize` values are reserved for non-visual data/algorithmic cases only.

## Ownership Model
The C++ composition root applies `WhatSonQmlContextBinder` before root QML load, registering important
ViewModels into `LV.ViewModels` and exposing required context objects. `Main.qml` then claims write ownership
for selected view IDs without re-registering those ViewModels.

## Why This Directory Is Important
If a runtime object exists in C++ but behaves incorrectly in the UI, this directory is usually where the mismatch becomes visible first.
