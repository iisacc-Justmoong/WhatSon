# `src/app/qml/view/content/editor/ContentsEditorSession.qml`

## Status
- Documentation phase: updated after the session-save policy moved into a dedicated C++ backend controller.
- Detail level: wrapper contract documented; persistence policy now lives in `ContentsEditorSessionController`.

## Source Metadata
- Source path: `src/app/qml/view/content/editor/ContentsEditorSession.qml`
- Source kind: QML view/component
- File name: `ContentsEditorSession.qml`
- Approximate line count: 50

## Fetch Sync Contract

- `ContentsEditorSession.qml` no longer owns the save/sync calculations itself.
- The file now wraps one `ContentsEditorSessionController` instance from `WhatSon.App.Internal 1.0`.
- The primary editor host (`ContentsDisplayView.qml`) now mounts `ContentsEditorSessionController` directly.
  This file remains as a compatibility shell so older QML call sites can still import `ContentsEditorSession.qml`
  without reintroducing save policy into JavaScript.
- Public editor-session methods such as `flushPendingEditorText()`, `scheduleEditorPersistence()`,
  `persistEditorTextImmediately(text)`, `markLocalEditorAuthority()`, and
  `requestSyncEditorTextFromSelection(...)` are now thin forwarding shims.
- The wrapper now also forwards `editorBoundNoteDirectoryPath`, and
  `requestSyncEditorTextFromSelection(...)` carries the selected note directory as its fourth argument.
  Same-id note packages are therefore disambiguated before the C++ controller applies same-note protection or rebind
  policy.
- `editorTextSynchronized()` remains the wrapper-level signal so existing QML callers do not need to learn the new
  backend type directly.

## QML Surface Snapshot
- Root type: `Item`

### Object IDs
- `editorSession`
- `sessionController`

### Required Properties
- None detected during scaffold generation.

### Signals
- `editorTextSynchronized`

## Persistence Guard Notes
- All persistence-guard decisions now live in `src/app/viewmodel/content/ContentsEditorSessionController.*`.
- The wrapper still exposes the same state properties so `ContentsDisplayView.qml`,
  `ContentsEditorTypingController.qml`, and `ContentsEditorSelectionController.qml` can keep their existing bindings.
- `ContentsEditorSelectionBridge` completion no longer reaches the wrapper through inline QML signal plumbing; the
  backend controller owns that connection directly and the wrapper only forwards `editorTextSynchronized()`.
- Any future change to same-note snapshot rejection, pending-save clearing, or agenda/callout normalization must be
  made in the C++ controller first instead of reintroducing that policy into wrapper JavaScript.

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
- The session must also treat a same `noteId` payload from a different `noteDirectoryPath` as a different mounted note,
  not as a safe same-note echo for the current editor session.
- The session must not infer a note id for model-sync requests from prior selection state when the caller omitted it.
- If filesystem/body lookup cannot provide note-owned text for the current selection, the session must still be able to
  bind an explicit empty-body fallback for that selected note.
- Rejecting a mismatched same-note model snapshot must not enqueue any extra persistence request for the current editor
  buffer.
  RAW/model reconciliation may reject the incoming snapshot for one turn, but that rejection must not become an
  editor-to-RAW overwrite trigger by itself.
- The QML wrapper must stay a forwarding shell; save/sync policy regressions should be fixed in
  `ContentsEditorSessionController` rather than re-expanded into JavaScript.
- If the editor buffer contains `<agenda date="yyyy-mm-dd">`, the first local persistence-staging turn after
  modification must rewrite that placeholder to current `YYYY-MM-DD`.
- If model/body sync includes empty `<task>` or empty `<callout>` blocks, editor session normalization must expose
  them as single-space anchor bodies so subsequent typing/Enter flow can still target those structures.
- Same-note model echoes that arrive while the local typing session is still authoritative should leave the protected
  editor buffer in place without scheduling an extra save just because the echo was rejected.
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
