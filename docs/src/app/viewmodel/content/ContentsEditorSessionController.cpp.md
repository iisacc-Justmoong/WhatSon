# `src/app/viewmodel/content/ContentsEditorSessionController.cpp`

## Implementation Notes
- The implementation keeps the old editor-session trace vocabulary (`editorSession.*`) so existing log analysis does not
  lose continuity even though the behavior moved from QML to C++.
- `ContentsEditorSelectionBridge` is consumed as a typed collaborator rather than through loosely-typed JavaScript
  property checks. That removes one layer of QML `undefined`-style branching from the save path.
- Agenda placeholder normalization stays delegated to `ContentsAgendaBackend.normalizeAgendaModifiedDate(...)`, but the
  decision of when that normalization applies is now controller-owned.
- Empty `<task>` / `<callout>` anchor normalization is now compiled regex work in C++, keeping save/model-sync text
  normalization off the QML hot path.
- The sync guard release uses a queued `QTimer::singleShot(0, ...)` turn so model-driven editor text changes still
  suppress the immediate follow-up typing echo without moving that scheduling rule back into QML.

## Regression Focus
- A same-note protected model snapshot must not reopen the reverse write path that previously allowed a rejected RAW
  snapshot to re-stage the current editor buffer back to persistence.
- A note switch with pending buffered edits must still flush the current note before the next selection binds, but the
  flush acceptance must stay distinct from a durable save completion.
