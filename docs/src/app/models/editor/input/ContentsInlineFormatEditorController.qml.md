# `src/app/models/editor/input/ContentsInlineFormatEditorController.qml`

## Responsibility

Owns non-visual state for `ContentsInlineFormatEditor.qml`.

The view remains the live `TextEdit` wrapper, while this controller owns selection snapshots, programmatic text-sync
policy, deferred edit dispatch, and native-composition guards.

## Contract

- Keeps the OS/Qt `TextEdit` as the ordinary input authority.
- Rejects programmatic cursor/text sync while `inputMethodComposing` or `preeditText` is active.
- Tracks local text edits so focused native-input sessions are not overwritten by host projection refreshes.
- Emits `textEdited(...)` only after committed text is available.

## Boundary

This object must not grow generic key-event handling for ordinary editing. Enter, Backspace, Delete, arrows, selection
extension, repeat, IME gestures, and pointer selection remain native `TextEdit` behavior.
