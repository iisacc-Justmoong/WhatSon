# `src/app/viewmodel/hierarchy/library/LibraryNoteMutationViewModel.hpp`

## Role
`LibraryNoteMutationViewModel` is a narrow facade for library note mutation commands and related signals.

## Interface Alignment
- The source viewmodel is now accepted as `QObject*` plus `ILibraryNoteMutationCapability`.
- This keeps mutation callers independent from the full `LibraryHierarchyViewModel` concrete surface.
