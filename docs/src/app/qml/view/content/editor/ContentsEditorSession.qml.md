# `src/app/qml/view/content/editor/ContentsEditorSession.qml`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/qml/view/content/editor/ContentsEditorSession.qml`
- Source kind: QML view/component
- File name: `ContentsEditorSession.qml`
- Approximate line count: 95

## QML Surface Snapshot
- Root type: `Item`

### Object IDs
- `editorSession`
- `bodySaveTimer`

### Required Properties
- None detected during scaffold generation.

### Signals
- `editorTextSynchronized`

## Persistence Guard Notes
- The session now distinguishes:
  - `pendingBodySave`: local edits waiting for debounce enqueue
  - `bodySaveInFlight`: a background save request already accepted by `ContentsEditorSelectionBridge`
- `acknowledgeQueuedEditorPersistence(noteId, text)` now clears only the pending-save/timer state and records the last
  accepted async payload.
- `acknowledgeSuccessfulEditorPersistence(noteId, text)` now runs only from bridge completion, so the session no longer
  assumes file persistence finished on the same turn that the editor requested it.
- Async completion for an older payload must not clear a newer `pendingBodySave`. The session now only clears
  `pendingBodySave` on synchronous fallback completion; async completion merely clears the tracked in-flight payload.
- Successful writes still do not revoke `localEditorAuthority` immediately. The session keeps that authority until the
  same note echoes the same body text back through `syncEditorTextFromSelection(...)`.
- `syncEditorTextFromSelection(...)` now drops local authority only when the note changes or when the model payload
  matches the current editor buffer exactly. This prevents newly created notes from accepting a stale empty
  `currentBodyText` snapshot during the first few keystrokes.
- `shouldAcceptModelBodyText(...)` therefore continues rejecting mismatched same-note model text while the editor still
  owns the current local buffer.
- Note-selection changes now enter through `requestSyncEditorTextFromSelection(...)`. When the user leaves a note with a
  pending unsaved body and the enqueue step fails, the session keeps the old note bound, preserves the pending body,
  and records the newly selected note as a deferred sync target instead of silently dropping the edit.
- Once enqueue succeeds, the session no longer blocks the note switch on the actual disk write. The previous note body
  continues saving asynchronously while the editor may already bind to the next note.
- If the async completion later reports a failure for the still-bound note and unchanged editor text, the session
  re-arms `pendingBodySave` so the debounce timer retries that body.

## Regression Checks
- Switching to another note while the current note still has a pending unsaved body must not overwrite `editorText`
  with the new note body unless the previous note body was at least accepted into the async save queue.
- Once the async enqueue succeeds, the editor must be able to bind to the latest requested note body without waiting
  for the physical file write to finish.
- If the user edits again while an older async save is still running, that older completion must not clear the newer
  pending debounce save.
- If the currently bound note's async save completion reports failure and the editor buffer still matches that queued
  payload, `pendingBodySave` must re-arm so the debounce retry path remains alive.

## Intended Detailed Sections
- Responsibility and business role
- Ownership and lifecycle
- Public API or externally observed bindings
- Collaborators and dependency direction
- Data flow and state transitions
- Error handling and recovery paths
- Threading, scheduling, or UI affinity constraints when relevant
- Extension points, invariants, and known complexity hotspots
- Test coverage and missing verification
## Authoring Notes For Next Pass
- Read the real implementation and adjacent headers before replacing this scaffold.
- Document concrete signals, slots, invokables, persistence side effects, and LVRS/QML bindings where applicable.
- Cross-link this file with peer modules in the same directory once the detailed pass begins.
