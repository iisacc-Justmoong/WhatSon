# `src/app/models/detailPanel/session/IWhatSonNoteHeaderSessionStore.hpp`

## Role
`IWhatSonNoteHeaderSessionStore` defines the session-backed note-header editing contract.

## Contract
- Session load/read access for `.wsnhead` state.
- Header mutations for project, bookmark, progress, folder, and tag operations.
- Tag mutations now explicitly include an assign/upsert path (`assignTag(...)`) in addition to indexed removal.
- `entryChanged(noteId)` notification for consumers that reflect header updates.
