# `src/app/models/editor/input/ContentsInlineFormatEditorController.cpp`

## Responsibility

Implements the C++ bridge for the active inline note editor controller.

## Current Behavior

- Keeps the `control` and `textInput` object handles directly in C++.
- Tracks local cursor/selection/text edits from the native `LV.TextEditor.editorItem` notify signals.
- Installs an event filter on the live `LV.TextEditor.editorItem` so rendered-overlay collapsed Backspace can be
  offered to the QML/C++ WYSIWYG mutation policy before Qt deletes hidden RAW tag bytes.
- Consults `ContentsEditorInputPolicyAdapter` directly for focused native-input programmatic text-sync decisions.
- Restores cursor and selection through the native text item, defers programmatic source replacement while composition
  is active, and dispatches committed text edits back to the editor surface.

## Boundary

This file is intentionally narrow. Removed structured block controllers must not be reintroduced here; block parsing,
rendering, and minimap projection belong to their dedicated model layers.
