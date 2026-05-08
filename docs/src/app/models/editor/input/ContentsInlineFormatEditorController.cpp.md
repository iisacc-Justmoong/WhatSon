# `src/app/models/editor/input/ContentsInlineFormatEditorController.cpp`

## Responsibility

Implements the C++ bridge for the active inline note editor controller.

## Current Behavior

- Keeps the `control` and `textInput` object handles directly in C++.
- Tracks local cursor/selection/text edits from the native `LV.TextEditor.editorItem` notify signals.
- Installs a narrow event filter on the native `LV.TextEditor.editorItem` for explicit tag-management shortcuts only.
  Ordinary Backspace/Delete, navigation, selection, repeat, and IME input remain on the native editor item;
  rendered-mode edits are converted to RAW source from the committed logical text delta by the QML/C++ WYSIWYG policy
  path.
- Consults `ContentsEditorInputPolicyAdapter` directly for focused native-input programmatic text-sync decisions.
- Delegates shortcut event normalization to `ContentsEditorInputPolicyAdapter`, keeping platform Command/Ctrl
  compensation, macOS Option text-symbol canonicalization, and the standard receive shape out of QML helper code.
- Restores cursor and selection through the native text item, defers programmatic source replacement while composition
  is active, and dispatches committed text edits back to the editor surface.
- Exposes native selection snapshots with explicit `logicalCursorPosition` and `logicalSelectionStart/End` fields.
  The legacy `cursorPosition` and `selectionStart/End` keys remain native logical coordinates in this controller; RAW
  source cursor/selection fields are exposed by `ContentsInlineFormatEditor.qml` after coordinate mapping.

## Boundary

This file is intentionally narrow. The event filter may dispatch only explicit tag-management shortcuts and must leave
ordinary text editing native. Removed structured block controllers must not be reintroduced here; block parsing,
rendering, and minimap projection belong to their dedicated model layers.
