# `src/app/qml`

## Role
This directory contains the root LVRS application shell and QML-side interaction surfaces.

The core rule in this directory is that visual composition belongs here, while persistence and mutation logic stay in C++ model-domain controllers or narrow bridge/helper objects under `src/app/models`.

## Important Files
- `Main.qml`: the root `LV.ApplicationWindow`, route shell, and root context-object consumer.
- `DesignTokens.qml`: QML-side design token aggregation.
- `contents/ContentsView.qml`: standalone Figma `ContentsView` frame that embeds the sibling gutter, editor, and
  minimap QML parts using `LV.Theme` tokens for colors, spacing, typography, and fixed rails.

## LVRS Token Rule
- QML view files must express reusable colors, transparency, spacing, typography, and fixed UI extents through
  `LV.Theme` tokens or token compositions.
- Direct visual literals such as `#...`, `Qt.rgba(...)`, `color: "transparent"`, `spacing: 0`, and raw
  `font.pixelSize` values are reserved for non-visual data/algorithmic cases only.

## Ownership Model
The C++ composition root applies `WhatSonQmlContextBinder` before root QML load, exposing required runtime objects as
LVRS context-object bindings. `Main.qml` consumes those objects directly and forwards explicit dependencies to child
views.

## Why This Directory Is Important
If a runtime object exists in C++ but behaves incorrectly in the UI, this directory is usually where the mismatch becomes visible first.
