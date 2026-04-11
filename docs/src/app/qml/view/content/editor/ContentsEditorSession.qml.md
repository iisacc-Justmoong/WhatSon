# `src/app/qml/view/content/editor/ContentsEditorSession.qml`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/qml/view/content/editor/ContentsEditorSession.qml`
- Source kind: QML view/component
- File name: `ContentsEditorSession.qml`
- Approximate line count: 121

## Fetch Sync Contract

- The session no longer owns a debounce timer.
- `pendingBodySave` now only tracks whether the currently bound editor buffer still has a staged-but-not-yet-confirmed
  filesystem persistence result for that same note/text pair.
- `scheduleEditorPersistence()` stages the latest editor snapshot immediately into the bridge's buffered fetch-sync
  boundary.
- During that staging turn, session now treats `date="yyyy-mm-dd"` in `<agenda ...>` as a modified-time placeholder
  and rewrites it to the current local `YYYY-MM-DD` token before enqueueing persistence.
- That placeholder normalization is delegated to `ContentsAgendaBackend.normalizeAgendaModifiedDate(...)`.
- The session no longer blocks note swaps on an immediate save acceptance path.
- `flushPendingEditorText()` is now only a best-effort final fetch request; it is no longer part of ordinary note-switch
  control flow.

## QML Surface Snapshot
- Root type: `Item`

### Object IDs
- `editorSession`

### Required Properties
- None detected during scaffold generation.

### Signals
- `editorTextSynchronized`

## Persistence Guard Notes
- The session no longer owns deferred note-switch state or queued-payload tracking.
- The session still only talks to `ContentsEditorSelectionBridge`, but that bridge now forwards completion from
  `ContentsEditorIdleSyncController`; the session does not own fetch timing, save, stat, or open-count behavior.
- Successful completion only clears `pendingBodySave` when the finished payload still matches the currently bound
  editor buffer.
- Async completion for an older payload must not clear a newer `pendingBodySave`.
- Agenda placeholder-date normalization is bound to modification staging, not passive model sync:
  - model echo application must not rewrite existing agenda dates
  - only local modified/persistence staging paths rewrite `yyyy-mm-dd`
- Successful writes still do not revoke `localEditorAuthority` immediately. The session keeps that authority until the
  bound note itself changes.
- `syncEditorTextFromSelection(...)` now drops local authority only when the note changes.
  Same-note echo payloads no longer clear that authority, so a later stale `currentBodyText` refresh cannot reclaim
  the live editor buffer one polling cycle after the correct body text already echoed back once.
- `typingIdleThresholdMs` and `lastLocalEditTimestampMs` now define a non-idle typing window.
- `shouldAcceptModelBodyText(...)` now rejects mismatched same-note model text while either:
  - that typing window is still active
  - the current local note body is still pending persistence completion
- `requestSyncEditorTextFromSelection(...)` now short-circuits same-note no-op sync requests and blocks same-note
  mismatched model snapshot application unless `shouldAcceptModelBodyText(...)` allows it.
- `syncEditorTextFromSelection(...)` now arms `syncingEditorTextFromModel` only when the body text actually changes, so
  same-note same-text sync turns no longer open a short-lived guard window.
- Note-selection changes now enter through `requestSyncEditorTextFromSelection(...)`. When the user leaves a note with a
  pending staged body, the session simply re-stages the latest current-note text and then binds the next note
  immediately. The previous note stays buffered in the fetch-sync controller instead of blocking the UI.
- Same-note model echoes that exactly match the current editor buffer now leave `pendingBodySave` untouched; only an
  actual success completion for that same note/text pair clears it.

## Regression Checks
- Switching to another note while the current note still has a pending staged body must not require an immediate save
  success path before the new note body binds.
- The previous note body must still remain recoverable through the buffered fetch-sync controller after that note switch.
- If the user edits again while an older async save is still running, that older completion must not clear the newer
  pending staged save.
- Same-note model echoes that only mirror the current text must not clear `pendingBodySave` ahead of the real
  persistence completion.
- A same-note model echo that exactly matches the current editor buffer must not revoke local authority and thereby make
  the next stale snapshot eligible to overwrite the live note body.
- While typing remains inside `typingIdleThresholdMs`, mismatched same-note model snapshots must not overwrite
  `editorText`.
- Non-changing same-note model snapshots must not arm a temporary model-sync guard that can suppress the next user
  `textEdited` processing turn.
- If the editor buffer contains `<agenda date="yyyy-mm-dd">`, the first local persistence-staging turn after
  modification must rewrite that placeholder to current `YYYY-MM-DD`.

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
