# `src/app/models/file/sync/WhatSonEditorRawPushController.cpp`

## Runtime Behavior

- `requestIdlePush(...)` records the latest editor document text and schedules an idle timer.
- `requestModifiedCountPush(...)` remains available for fallback trigger tests, but live note input now writes RAW
  directly through `NoteEditorDocumentSession` from LVRS `textEdited(text)`.
- If an idle sync trigger arrives while a modified-count push is already pending, the pending user-edit payload wins.
  This prevents a stale session-file sync notification from replacing the first Backspace/delete snapshot before it is
  converted back to RAW source.
- The controller only orders fallback trigger payloads. `NoteEditorDocumentSession` is responsible for direct RAW input
  writes and for ignoring stale sync-finished payloads after the user's last action.
- `pushBeforeNoteDeparture(...)` stops any pending idle push and flushes the latest known surface payload immediately.
  If no live payload is pending for that file, the callback is invoked without editor text so the session can fall back
  to its note-departure persistence policy.
- `flushPendingPush()` executes the most recent pending push once.
- `discardPendingPushForFile(...)` drops a pending payload for a session file after another owner has already made an
  authoritative write. It also resets modified-count tracking for that file so the next live edit can schedule normally.

## Error Handling

- Empty paths are ignored for scheduled triggers and treated as no-op success for note departure.
- Discard requests for an empty path or a file without a pending payload return `false` without emitting push signals.
- If no callback is installed, `rawPushFinished(...)` reports failure with a clear message.

## Boundary

- The controller never writes RAW itself. It invokes the callback supplied by `NoteEditorDocumentSession`.
