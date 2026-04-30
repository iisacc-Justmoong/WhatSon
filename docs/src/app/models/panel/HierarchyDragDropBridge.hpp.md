# `src/app/models/panel/HierarchyDragDropBridge.hpp`

## Role
This header defines the bridge used by QML hierarchy views for drag/drop-related operations.

It exposes three ideas to QML.
- Whether reorder is supported.
- Whether note-drop assignment is supported.
- Which hierarchy item key is currently selected.

## Public Surface
- Properties:
  - `hierarchyController`
  - `reorderContractAvailable`
  - `noteDropContractAvailable`
  - `selectedItemKey`
- Invokables:
  - `applyHierarchyReorder(...)`
  - `canAcceptNoteDrop(...)`
  - `canAcceptNoteDropList(...)`
  - `assignNoteToFolder(...)`
  - `assignNotesToFolder(...)`

## Design Constraint
This bridge only understands the generic hierarchy interface plus capability interfaces. It should not grow domain-specific policy. Domain-specific accept/reject logic belongs in the concrete hierarchy controller capability implementation.
