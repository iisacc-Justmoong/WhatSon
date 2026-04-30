# `src/app/qml/contents`

## Role
This directory hosts standalone contents-surface view artifacts that mirror Figma content frames before they are wired
into the runtime editor stack.

## Current Files
- `ContentsView.qml`: Figma node `155:4561` root frame and LVRS layout host.
- `Gutter.qml`: line-number rail and change/conflict markers for Figma node `155:5345`.
- `EditorView.qml`: read-only text projection for Figma node `155:5352`.
- `Minimap.qml`: minimap row rail for Figma node `352:8626`.

## Boundary
- The view is presentation-only.
- It does not own note persistence, parser state, or editor mutation authority.
- Every QML file imports LVRS and expresses colors, spacing, typography, and fixed rails through `LV.Theme` tokens.
- Figma dimensions are preserved through token compositions instead of literal pixel values.

## Tests
- `test/cpp/suites/qml_contents_view_tests.cpp` locks the four-file composition contract and the read-only editor
  projection boundary.
