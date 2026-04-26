# `src/app/models/editor/input/ContentsInlineFormatEditorController.qml`

## Responsibility

Owns non-visual state for `ContentsInlineFormatEditor.qml`.

The view remains the live `TextEdit` wrapper, while this controller owns selection snapshots, programmatic text-sync
policy, deferred edit dispatch, native-composition guards, and the explicit macOS Option-word navigation fallback.

## Contract

- Keeps the OS/Qt `TextEdit` as the ordinary input authority.
- Rejects programmatic cursor/text sync while `inputMethodComposing` or `preeditText` is active.
- Tracks local text edits so focused native-input sessions are not overwritten by host projection refreshes.
- Handles `Option+Left/Right` and `Option+Shift+Left/Right` on macOS by moving/selecting to word boundaries on the live
  `TextEdit`.
- Emits `textEdited(...)` only after committed text is available.

## Boundary

This object may process explicit platform text-navigation chords that Qt Quick does not guarantee, but it must not grow
generic key-event handling for ordinary editing. Enter, Backspace, Delete, repeat, IME gestures, and pointer selection
remain native `TextEdit` behavior.
