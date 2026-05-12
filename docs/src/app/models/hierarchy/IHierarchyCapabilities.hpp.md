# `src/app/models/hierarchy/IHierarchyCapabilities.hpp`

## Role
This header collects opt-in hierarchy capability interfaces.

## Recent Addition
- `IHierarchyReorderCapability` exposes full-node reorder replay and an optional direct move-event helper. The sidebar
  drag path uses full-node replay from `LV.Hierarchy.model`, while concrete controllers can still expose
  `applyHierarchyMove(...)` for explicit targeted move callers.
- `ILibraryNoteMutationCapability` now defines note creation, folder clearing, and note deletion as a small collaboration contract.
- `LibraryNoteMutationController` uses this capability instead of the full `LibraryHierarchyController` type.
