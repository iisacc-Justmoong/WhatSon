# `src/app/qml/contents/EditorView.qml`

## Role
`EditorView.qml` renders the Figma text editor region for node `155:5352`.

## Contract
- Imports LVRS and uses LVRS metric, typography, and selection color tokens.
- Exposes `editorText` and `textColor` for root-frame binding.
- Keeps the `TextEdit` read-only so this Figma artifact cannot become a write-authoritative note-body editor.
- Does not install custom key handlers or input-method calls.

## UI
- Uses a native `TextEdit` for text projection and selection.
- Keeps the Figma horizontal inset through `LV.Theme.gap16`.
