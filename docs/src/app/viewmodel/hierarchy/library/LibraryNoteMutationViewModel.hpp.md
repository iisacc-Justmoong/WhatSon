# `src/app/viewmodel/hierarchy/library/LibraryNoteMutationViewModel.hpp`

## Role
`LibraryNoteMutationViewModel` is a narrow facade for library note mutation commands and related signals.

## Interface Alignment
- The source viewmodel is now accepted as `QObject*` plus `ILibraryNoteMutationCapability`.
- This keeps mutation callers independent from the full `LibraryHierarchyViewModel` concrete surface.
- Single-note commands (`deleteNoteById(...)`, `clearNoteFoldersById(...)`) are still exposed directly for focused-note
  flows.
- Batch note-list actions can now route through `deleteNotesByIds(...)` and `clearNoteFoldersByIds(...)`, which accept
  QML arrays and fan them back into the source capability one note id at a time.
