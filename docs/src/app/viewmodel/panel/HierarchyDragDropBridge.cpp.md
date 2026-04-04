# `src/app/viewmodel/panel/HierarchyDragDropBridge.cpp`

## Role
This implementation adapts drag/drop operations from QML into capability-based hierarchy mutations.

## Core Behavior
- `setHierarchyViewModel(...)` verifies the `View -> ViewModel` edge through the policy layer.
- The bridge watches both hierarchy node changes and selection changes.
- `refreshContractState()` caches whether reorder and note-drop are available.
- `refreshSelectedItemKey()` extracts the currently selected node key from the normalized hierarchy model.

## Reorder Path
`applyHierarchyReorder(...)` succeeds only when:
- a hierarchy viewmodel is present
- the viewmodel implements `IHierarchyReorderCapability`
- reorder support is enabled
- the incoming node list is not empty

The bridge forwards the caller-provided active item key when available and otherwise falls back to the currently selected key.

## Note-Drop Path
`canAcceptNoteDrop(...)` and `assignNoteToFolder(...)` both:
- require `IHierarchyNoteDropCapability`
- normalize and trim the note ID
- reject negative indices and empty note IDs before delegating

This keeps the QML layer free from repeated sanitation logic and ensures that malformed drag payloads fail early.

For note-list multi-selection, `canAcceptNoteDropList(...)` and `assignNotesToFolder(...)` normalize/deduplicate a
QML-provided note-id array and replay the same capability contract across the selected notes.
