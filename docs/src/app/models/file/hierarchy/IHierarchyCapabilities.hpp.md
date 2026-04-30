# `src/app/models/file/hierarchy/IHierarchyCapabilities.hpp`

## Role
This header collects opt-in hierarchy capability interfaces.

## Recent Addition
- `ILibraryNoteMutationCapability` now defines note creation, folder clearing, and note deletion as a small collaboration contract.
- `LibraryNoteMutationController` uses this capability instead of the full `LibraryHierarchyController` type.
