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
- The hidden mobile `ContentsInlineFormatEditor` is now also idled during structured-flow editing by clearing its
  document binding, disabling the control, and disconnecting its layout/cursor observers until fallback mode returns.
- Mobile note-open reconcile now also uses a deferred request/complete cycle:
  - selection changes queue one reconcile attempt per note
  - `selectionBridge.viewSessionSnapshotReconciled(...)` closes that pending state
  - timer-driven snapshot polling no longer performs synchronous RAW note reads on the UI thread
