# `src/app/viewmodel/hierarchy/library/LibraryNoteMutationViewModel.hpp`

## Role
This header defines a small QObject facade for note-mutation operations in the library domain.

The class exists so callers that only need note creation or note deletion do not have to hold the full `LibraryHierarchyViewModel` surface.

## Public API
- `setSourceViewModel(...)`: inject the backing `LibraryHierarchyViewModel`.
- `createEmptyNote()`
- `clearNoteFoldersById(...)`
- `deleteNoteById(...)`

## Signals
- `sourceViewModelChanged()`: emitted when the backing source is swapped or destroyed.
- `noteDeleted(...)`
- `emptyNoteCreated(...)`
- `hubFilesystemMutated()`

These mutation outcome signals are forwarded from the source viewmodel so callers can subscribe without knowing the wider library hierarchy type.

## Expected Usage
This object is primarily useful for:
- keyboard shortcut handlers
- lightweight panel interactions
- future write-owned LVRS view bindings that should not see unrelated hierarchy APIs
