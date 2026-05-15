# `src/app/models/file/sync/WhatSonEditorRawPullController.cpp`

## Runtime Behavior

- `requestNoteEntryPull(...)` normalizes the note id/path and invokes the pull callback with the `note-entry` reason.
- `requestNoteOpenPull(...)` normalizes the note id/path and invokes the pull callback with the `note-open` reason.
- `setActiveNoteForIdlePull(...)` records the currently open note and starts a single-shot idle timer.
- `recordUserActivity()` restarts that timer; when no activity is observed for `idlePullIntervalMs` milliseconds,
  `requestActiveIdlePull()` invokes the pull callback with the `idle` reason and schedules the next idle interval.
- The default idle pull interval is 5000 ms, matching the editor sync policy for idle opened-note refresh.
- Empty note ids or paths are ignored and return sequence `0`.
- A successful callback must return a non-zero load sequence.

## Error Handling

- If no callback is installed, `rawPullFinished(...)` reports failure with a clear message.
- If the callback returns `0` without an error, the controller emits the same fallback message.

## Boundary

- The controller never reads RAW itself. It invokes the callback supplied by `NoteEditorDocumentSession`.
- Timestamp comparison and editor-session application remain in `NoteEditorDocumentSession`; this controller only owns
  idle/open/entry pull scheduling.
