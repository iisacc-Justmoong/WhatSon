# `src/app/models/hierarchy/IHierarchyCapabilities.hpp`

## Role
This header collects opt-in hierarchy capability interfaces.

## Recent Addition
- `IHierarchyReorderCapability` exposes both full-node reorder replay and direct LVRS move-event replay. Controllers
  that persist nested folders should prefer `applyHierarchyMove(...)` for drag/drop, because it receives the single
  source index, target index, and target depth that LVRS calculated for the visible move.
- `ILibraryNoteMutationCapability` now defines note creation, folder clearing, and note deletion as a small collaboration contract.
- `LibraryNoteMutationController` uses this capability instead of the full `LibraryHierarchyController` type.
