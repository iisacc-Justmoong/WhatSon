# `src/app/models/content/ContentsEditorSessionController.cpp`

## Implementation Notes
- The implementation keeps the old editor-session trace vocabulary (`editorSession.*`) so existing log analysis does not
  lose continuity even though the behavior moved from QML to C++.
- `ContentsEditorSelectionBridge` is consumed as a typed collaborator rather than through loosely-typed JavaScript
  property checks. That removes one layer of QML `undefined`-style branching from the save path.
- Creation-time collaborator wiring is traced for `selectionBridge`, including the collaborator pointer, runtime class
  name, and QML `objectName`, so editor bootstrap logs show which concrete object was attached before the first
  note-body mount attempt.
- Agenda/task and callout source is no longer normalized by the session. `<agenda><task>...</task></agenda>` and
  `<callout>...</callout>` are ordinary transparent paired tags, so save/model-sync payloads preserve the authored RAW
  wrappers exactly.
- Same-note model sync now keeps honoring `localEditorAuthority` even after the typing-idle window has elapsed, so a
  delayed RAW refresh cannot reclaim editor-owned text merely because the user paused typing for a moment.
- The session controller now also tracks `editorBoundNoteDirectoryPath`.
  Same `noteId` payloads are only considered the same mounted note while both the note id and the normalized
  `.wsnote` directory match; if the directory differs, the controller performs a full rebind instead of protecting the
  old package under same-note local-authority rules.
- The sync guard release uses a queued `QTimer::singleShot(0, ...)` turn so model-driven editor text changes still
  suppress the immediate follow-up typing echo without moving that scheduling rule back into QML.

## Regression Focus
- A same-note protected model snapshot must not reopen the reverse write path that previously allowed a rejected RAW
  snapshot to re-stage the current editor buffer back to persistence.
- A same-note RAW/model snapshot that differs from the current editor text must stay rejected while the selected note
  still has local editor authority, even when `pendingBodySave == false`.
- A note switch with pending buffered edits must still flush the current note before the next selection binds, but the
  flush acceptance must stay distinct from a durable save completion.
- A same `noteId` model snapshot that resolves to a different `noteDirectoryPath` must be treated as a package switch,
  not as a same-note echo from the currently mounted editor session.
