# `src/app/models/file/sync/WhatSonEditorRawPushController.cpp`

## Runtime Behavior

- `requestIdlePush(...)` records the latest verified RAW source text and schedules an idle timer. The default interval is
  nonzero, so idle sync never executes in the same event turn as `readFinished`/programmatic document application.
- `requestModifiedCountPush(...)` records the latest verified RAW source text produced by
  `NoteEditorDocumentSession`.
- If an idle sync trigger arrives while a modified-count push is already pending, the pending user-edit RAW payload wins.
  This prevents a stale session-file sync notification from replacing the first Backspace/delete snapshot before it is
  persisted.
- The controller only orders RAW trigger payloads. `NoteEditorDocumentSession` is responsible for editor-document to RAW
  conversion, unsafe empty-payload rejection, and ignoring stale sync-finished payloads after the user's last action.
- `pushBeforeNoteDeparture(...)` stops any pending idle push and flushes the latest known RAW payload immediately. If no
  RAW payload is pending for that file, it is a no-op; note departure must use the session's active RAW source rather
  than re-reading a stale editor session file.
- `flushPendingPush()` executes the most recent pending push once.
- `discardPendingPushForFile(...)` drops a pending payload for a session file after another owner has already made an
  authoritative write. It also resets modified-count tracking for that file so the next live edit can schedule normally.
- The controller does not calculate the diff itself. It preserves trigger ordering, while the editor session and
  coordinator carry the loaded RAW base into the persistence worker where `file/diff` applies the body delta.

## Error Handling

- Empty paths are ignored for scheduled triggers and treated as no-op success for note departure.
- Discard requests for an empty path or a file without a pending payload return `false` without emitting push signals.
- If no callback is installed, `rawPushFinished(...)` reports failure with a clear message.

## Boundary

- The controller never converts editor HTML or writes RAW itself. It invokes the callback supplied by
  `NoteEditorDocumentSession` with already verified RAW source text.
