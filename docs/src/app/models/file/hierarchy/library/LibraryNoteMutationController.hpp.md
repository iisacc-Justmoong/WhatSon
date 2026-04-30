# `src/app/models/file/hierarchy/library/LibraryNoteMutationController.hpp`

## Role
`LibraryNoteMutationController` is a narrow facade for library note mutation commands and related signals.

## Interface Alignment
- The source controller is now accepted as `QObject*` plus `ILibraryNoteMutationCapability`.
- This keeps mutation callers independent from the full `LibraryHierarchyController` concrete surface.
- Single-note commands (`deleteNoteById(...)`, `clearNoteFoldersById(...)`) are still exposed directly for focused-note
  flows.
- Batch note-list actions can now route through `deleteNotesByIds(...)` and `clearNoteFoldersByIds(...)`, which accept
  QML arrays and fan them back into the source capability one note id at a time.
