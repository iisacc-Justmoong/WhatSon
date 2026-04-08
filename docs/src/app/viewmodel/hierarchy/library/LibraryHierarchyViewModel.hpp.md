# `src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp`

## Role
`LibraryHierarchyViewModel` owns library hierarchy state, note list projection, and library note mutations.

## Interface Alignment
- Still implements the hierarchy capability set.
- Now also implements `ILibraryNoteMutationCapability` so mutation-only consumers can avoid the full concrete type.
- System calendar injection now depends on `ISystemCalendarStore`.
- Exposes an indexed-note snapshot accessor so adjacent runtime collaborators can project the current library note
  metadata without reparsing the hub from disk.
