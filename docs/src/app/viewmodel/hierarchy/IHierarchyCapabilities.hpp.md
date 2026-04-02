# `src/app/viewmodel/hierarchy/IHierarchyCapabilities.hpp`

## Role
This header collects opt-in hierarchy capability interfaces.

## Recent Addition
- `ILibraryNoteMutationCapability` now defines note creation, folder clearing, and note deletion as a small collaboration contract.
- `LibraryNoteMutationViewModel` uses this capability instead of the full `LibraryHierarchyViewModel` type.
