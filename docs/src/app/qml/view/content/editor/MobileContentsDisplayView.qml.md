# `src/app/qml/view/content/editor/MobileContentsDisplayView.qml`

## Responsibility
Mobile content editor host.

## Structured Document Flow
- Mobile now mirrors the desktop structured-flow editor path for notes that contain `agenda`, `callout`, or `break`
  blocks.
- `ContentsStructuredDocumentFlow.qml` becomes the visible editor surface for those notes, while the legacy inline
  editor remains the fallback for purely plain-text notes.
- Structured block rewrites route through `applyDocumentSourceMutation(...)` so mobile keeps the same RAW persistence
  contract as desktop, but they no longer force an immediate full legacy presentation rebuild while structured-flow
  editing remains active.
- While structured-flow mode is active, mobile also disconnects the hidden legacy agenda/callout overlay models so note
  open does not pay for both rendering paths at once.
- The legacy mobile `ContentsInlineFormatEditor` now unloads entirely during structured-flow editing and is recreated only
  when the fallback plain-text path becomes active again.
- Mobile keeps a lightweight proxy object behind the shared `contentEditor` reference so existing geometry/focus helpers
  remain null-safe while the legacy editor instance is absent.
- Mobile note-open reconcile now also uses a deferred request/complete cycle:
  - selection changes queue one reconcile attempt per note
  - `selectionBridge.viewSessionSnapshotReconciled(...)` closes that pending state
  - timer-driven snapshot polling no longer performs synchronous RAW note reads on the UI thread
- Mobile polling now also skips overlapping reconcile requests for the same selected note while one worker fetch is
  already pending.
- Mobile body-sync and correction/reconcile refresh work now also routes through
  `editorSession.editorTextSynchronized`, removing duplicate minimap/presentation/gutter refresh scheduling from
  multiple completion handlers.
- Mobile selection-driven editor sync now also collapses initial mount, `selectedNoteIdChanged`, and
  `selectedNoteBodyTextChanged` into one queued `scheduleSelectionModelSync(...)` pass per event-loop turn, and mobile
  visibility re-entry now reuses that same helper instead of scheduling a parallel note-open refresh path. One
  note-open transition therefore no longer replays the same editor/session refresh logic twice from separate handlers.
- The shared mobile selection-sync helper also keeps the `requestSyncEditorTextFromSelection(...)` result in one local
  gate before fallback refresh scheduling, matching the desktop deferred sync path while staying within QML parser
  identifier constraints.
- Mobile note-open now also waits for the selected note body lazy-load to finish before pushing selection state into
  the live editor session.
  While loading is pending, mobile disables the editor viewport, keeps the previous buffer intact, and shows the same
  loading overlay contract as desktop instead of syncing an empty placeholder body.
- Mobile note-open/model sync now also keeps the structured-flow surface mounted while
  `ContentsStructuredBlockRenderer.renderPending` is true, allowing agenda/callout projection to finish on a worker
  thread instead of blocking the first note-open frame on the UI thread.
- Mobile note snapshot polling also pauses during `selectedNoteBodyLoading`, so the first lazy note-open read is not
  interleaved with a second same-note refresh probe.
