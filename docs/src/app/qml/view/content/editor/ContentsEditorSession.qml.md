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
- `scheduleEditorPersistence()` now exists as the explicit buffered-retry entrypoint for the session and returns
  whether that staging request was actually accepted by the bridge.
- `persistEditorTextImmediately(text)` now marks the current note dirty and forwards that explicit payload through
  `enqueueEditorPersistence(noteId, bodyText, true)`, requesting one immediate fetch enqueue attempt instead of
  waiting for the periodic fetch clock.
- During that staging turn, session now treats `date="yyyy-mm-dd"` in `<agenda ...>` as a modified-time placeholder
  and rewrites it to the current local `YYYY-MM-DD` token before enqueueing persistence.
- That placeholder normalization is delegated to `ContentsAgendaBackend.normalizeAgendaModifiedDate(...)`.
- Session also normalizes empty structured blocks into cursor-reachable anchors before sync/persistence staging:
  - `<task ...></task>` -> `<task ...> </task>`
  - `<callout></callout>` -> `<callout> </callout>`
  - this keeps existing empty agenda/callout tags editable through logical/source offset mapping.
- The session no longer blocks note swaps on an immediate save acceptance path.
- `flushPendingEditorText()` remains a best-effort immediate fetch request and is now also reused by note-switch
  control flow when the current note still has a pending body save.
- Ordinary typing and source-rewrite helpers now also use that explicit immediate fetch path by default, so the live
  editor session can push `.wsnbody` updates without waiting for a host-side deferred-persistence escape hatch.
- If a note switch arrives while `pendingBodySave` is still true, `requestSyncEditorTextFromSelection(...)` now issues
  `flushPendingEditorText()` first and refuses to bind the next note unless that current-note snapshot was accepted into
  the persistence queue.
- `requestSyncEditorTextFromSelection(...)` now also refuses to bind incoming model text when the supplied body owner
  note id does not match the selected note id.
- When a mismatched same-note model snapshot is rejected, that same entrypoint now re-issues
  `persistEditorTextImmediately(currentText)` for the live buffer instead of downgrading the current note back to
  deferred staging.
- That entrypoint no longer falls back to any session-stored selected note id when callers omit one.
  Callers must pass explicit note ownership now.

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
  Same-note echo payloads no longer clear that authority, so a later stale selected-note snapshot cannot reclaim the
  live editor buffer one polling cycle after the correct body text already echoed back once.
- `typingIdleThresholdMs` and `lastLocalEditTimestampMs` now define a non-idle typing window.
- `shouldAcceptModelBodyText(...)` now rejects mismatched same-note model text while either:
  - that typing window is still active
  - the current local note body is still pending persistence completion
- `requestSyncEditorTextFromSelection(...)` now short-circuits same-note no-op sync requests and blocks same-note
  mismatched model snapshot application unless `shouldAcceptModelBodyText(...)` allows it.
- The same entrypoint also now rejects body payloads whose ownership note id does not match the selected note.
  The session therefore no longer has to guess whether a given body string belongs to the note the user just picked.
- Because the session no longer carries an alternate selected-note fallback for that API, a missing note id is treated
  as a hard rejection instead of being silently rebound to whatever note happened to be current.
- `syncEditorTextFromSelection(...)` now arms `syncingEditorTextFromModel` only when the body text actually changes, so
  same-note same-text sync turns no longer open a short-lived guard window.
- Note-selection changes now enter through `requestSyncEditorTextFromSelection(...)`. When the user leaves a note with a
  pending staged body, the session simply re-stages the latest current-note text and then binds the next note
  immediately once that staging request is accepted. The previous note stays buffered in the fetch-sync controller
  instead of blocking the UI.
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
- A note switch must not clear `pendingBodySave` and bind the next note when the current-note staging request was not
  accepted by the persistence bridge.
- Leaving a note with pending edits should attempt the immediate fetch path before the new note binds, reducing the
  window where the old note exists only in the in-memory buffer.
- Immediate session persistence must forward the caller-provided text payload into the flush path, not ignore it and
  downgrade the write to deferred staging only.
- The session must not bind model text for note `B` unless the upstream bridge explicitly proves that the incoming body
  payload also belongs to note `B`.
- The session must not infer a note id for model-sync requests from prior selection state when the caller omitted it.
- If filesystem/body lookup cannot provide note-owned text for the current selection, the session must still be able to
  bind an explicit empty-body fallback for that selected note.
- Rejecting a mismatched same-note model snapshot must not quietly push the live editor buffer back onto the deferred
  staging path; the session must request an immediate `.wsnbody` flush for that current text instead.
- If the editor buffer contains `<agenda date="yyyy-mm-dd">`, the first local persistence-staging turn after
  modification must rewrite that placeholder to current `YYYY-MM-DD`.
- If model/body sync includes empty `<task>` or empty `<callout>` blocks, editor session normalization must expose
  them as single-space anchor bodies so subsequent typing/Enter flow can still target those structures.
- Same-note model echoes that arrive while the local typing session is still authoritative should re-stage the current
  buffer through deferred persistence instead of forcing an immediate file flush on the echo turn.
- Immediate persistence remains a dedicated flush path for blur/note-switch settlement and should not be the default
  write path for ordinary in-note typing.

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
