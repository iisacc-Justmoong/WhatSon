# `src/app/models/file/sync/WhatSonEditorRawPushController.cpp`

## Runtime Behavior

- `requestIdlePush(...)` records the latest editor document text and schedules an idle timer.
- `requestModifiedCountPush(...)` records the latest editor document text only when the supplied modified count is
  greater than the last count seen for that editor session file.
- If an idle sync trigger arrives while a modified-count push is already pending, the pending user-edit payload wins.
  This prevents a stale session-file sync notification from replacing the first Backspace/delete snapshot before it is
  converted back to RAW source.
- The controller only orders trigger payloads. `NoteEditorDocumentSession` is responsible for promoting the latest
  modified-count payload to the active session source immediately and for making idle pushes persist that source when a
  stale sync-finished payload arrives after the user's last action. The same session-side policy handles
  note-departure fallback when the mounted session file is older than the active source.
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
