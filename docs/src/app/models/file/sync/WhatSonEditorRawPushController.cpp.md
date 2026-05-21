# `src/app/models/file/sync/WhatSonEditorRawPushController.cpp`

## Runtime Behavior

- `requestIdlePush(...)` records the latest editor document text and schedules an idle timer.
- `requestModifiedCountPush(...)` records the latest editor document text only when the supplied modified count is
  greater than the last count seen for that editor session file.
- If an idle sync trigger arrives while a modified-count push is already pending, the pending user-edit payload wins.
  This prevents a stale session-file sync notification from replacing the first Backspace/delete snapshot before it is
  converted back to RAW source.
- `pushBeforeNoteDeparture(...)` stops any pending idle push and flushes the latest known surface payload immediately.
  If no live payload is pending for that file, the callback is invoked without editor text so the session can fall back
  to the saved surface file.
- `flushPendingPush()` executes the most recent pending push once.

## Error Handling

- Empty paths are ignored for scheduled triggers and treated as no-op success for note departure.
- If no callback is installed, `rawPushFinished(...)` reports failure with a clear message.

## Boundary

- The controller never writes RAW itself. It invokes the callback supplied by `NoteEditorDocumentSession`.
