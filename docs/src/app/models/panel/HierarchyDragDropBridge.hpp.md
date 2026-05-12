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
  - `applyHierarchyReorder(...)`, accepting either a C++ `QVariantList` or the JS-array-shaped `var` model handed back
    by QML after LVRS has applied a hierarchy item move.
  - `applyHierarchyMove(...)`, forwarding the LVRS `listItemMoved` event payload (`fromIndex`, `toIndex`, `depth`,
    and the active item key) directly to the active reorder-capable controller.
  - `canAcceptNoteDrop(...)`
  - `canAcceptNoteDropList(...)`
  - `assignNoteToFolder(...)`
  - `assignNotesToFolder(...)`

## Design Constraint
This bridge only understands the generic hierarchy interface plus capability interfaces. It should not grow domain-specific policy. Domain-specific accept/reject logic belongs in the concrete hierarchy controller capability implementation.
