# `src/app/models/panel/HierarchyDragDropBridge.cpp`

## Role
This implementation adapts drag/drop operations from QML into capability-based hierarchy mutations.

## Core Behavior
- `setHierarchyController(...)` verifies the `View -> Controller` edge through the policy layer. It remains rebindable
  after the architecture lock because QML creates this bridge at view runtime and switches it with the active hierarchy
  domain.
- The bridge watches both hierarchy node changes and selection changes.
- `refreshContractState()` caches whether reorder and note-drop are available.
- `refreshSelectedItemKey()` extracts the currently selected node key from the normalized hierarchy model.

## Reorder Path
`applyHierarchyReorder(...)` succeeds only when:
- a hierarchy controller is present
- the controller implements `IHierarchyReorderCapability`
- reorder support is enabled
- the incoming node list is not empty

The incoming model may be either a C++ `QVariantList` or a QML/JS array variant from `LV.Hierarchy.model`; the bridge
normalizes both forms before calling the controller. The bridge forwards the caller-provided active item key when
available and otherwise falls back to the currently selected key.

`applyHierarchyMove(...)` remains available as a low-level targeted move helper for controllers that want to replay a
single visible move. The sidebar's normal LVRS drag path uses `applyHierarchyReorder(...)` instead, because
`LV.Hierarchy` has already applied the visible tree move to its model before emitting `onListItemMoved`.

## Note-Drop Path
`canAcceptNoteDrop(...)` and `assignNoteToFolder(...)` both:
- require `IHierarchyNoteDropCapability`
- normalize and trim the note ID
- reject negative indices and empty note IDs before delegating

This keeps the QML layer free from repeated sanitation logic and ensures that malformed drag payloads fail early.

For note-list multi-selection, `canAcceptNoteDropList(...)` and `assignNotesToFolder(...)` normalize/deduplicate a
QML-provided note-id array and replay the same capability contract across the selected notes.
`assignNotesToFolder(...)` is the bridge-level commit used when a `ListBarLayout` note-list delegate is dropped on a
folder row in `SidebarHierarchyView`; the concrete hierarchy controller remains responsible for writing the note header
folder binding and refreshing derived note lists.
