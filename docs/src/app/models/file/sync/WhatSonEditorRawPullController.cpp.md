# `src/app/models/file/sync/WhatSonEditorRawPullController.cpp`

## Runtime Behavior

- `requestNoteEntryPull(...)` normalizes the note id/path and invokes the pull callback with the `note-entry` reason.
- `requestNoteOpenPull(...)` normalizes the note id/path and invokes the pull callback with the `note-open` reason.
- Empty note ids or paths are ignored and return sequence `0`.
- A successful callback must return a non-zero load sequence.

## Error Handling

- If no callback is installed, `rawPullFinished(...)` reports failure with a clear message.
- If the callback returns `0` without an error, the controller emits the same fallback message.

## Boundary

- The controller never reads RAW itself. It invokes the callback supplied by `NoteEditorDocumentSession`.
