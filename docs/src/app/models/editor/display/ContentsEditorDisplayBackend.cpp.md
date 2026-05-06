# `src/app/models/editor/display/ContentsEditorDisplayBackend.cpp`

## Responsibility

Implements the live note-editor display backend used by `ContentViewLayout.qml`.

## Key Behavior

- Attaches the owned `ContentsEditorSessionController` to `NoteActiveStateTracker` when present and clears that
  attachment on teardown.
- Synchronizes the editor session from active-note state first, then falls back to the note-list model snapshot.
- Commits editor changes by mutating the RAW session text and forwarding the same RAW body to the active content
  controller's `saveCurrentBodyText(QString)` method.
- Re-publishes session text into `ContentsEditorPresentationProjection` and `ContentsStructuredBlockRenderer`, then
  feeds renderer-owned block metadata into `ContentsBodyResourceRenderer`.
- Emits `editorViewportResetRequested()` only when a requested refresh changes the bound note identity
  (`noteId`/`noteDirectoryPath`). Same-note body refreshes, save/reconcile entry updates, and index refresh signals
  preserve the editor viewport so typing near the bottom cannot yank the document back to the first line.
- Owns resource-tag mutation callbacks and inline-resource HTML replacement so QML only binds the rendered output.
- Owns minimap layout metrics while `ContentViewLayout.qml` supplies LVRS token values and renders the resolved rail.
