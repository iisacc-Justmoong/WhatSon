# `src/app/models/panel`

## Role
This directory contains QObject bridge types that translate generic or QML-facing panel behavior into narrow controller interactions.

These classes are not the core domain state. They are adaptation layers between:
- generic QML components
- dedicated hierarchy or content controllers
- architecture policy checks

## Important Bridge Types
- `HierarchyInteractionBridge`: rename, create, delete, and expansion access through capabilities.
- `HierarchyDragDropBridge`: reorder and note-drop access through capabilities.
- `FocusedNoteDeletionBridge`: focused-note deletion helper.
- `NoteListModelContractBridge`: dynamic note-list search/selection contract adapter used by `ListBarLayout.qml`.
- `PanelController` and `PanelControllerRegistry`: panel-specific controller routing and hook dispatch.

## Why This Layer Exists
Without these bridge objects, QML components would either:
- depend directly on large concrete controllers, or
- duplicate capability detection and guard logic in JavaScript.

The bridge layer keeps capability checks, ownership assumptions, and architecture-policy verification in one place.
