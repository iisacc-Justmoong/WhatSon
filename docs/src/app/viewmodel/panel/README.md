# `src/app/viewmodel/panel`

## Role
This directory contains QObject bridge types that translate generic or QML-facing panel behavior into narrow viewmodel interactions.

These classes are not the core domain state. They are adaptation layers between:
- generic QML components
- dedicated hierarchy or content viewmodels
- architecture policy checks

## Important Bridge Types
- `HierarchyInteractionBridge`: rename, create, delete, and expansion access through capabilities.
- `HierarchyDragDropBridge`: reorder and note-drop access through capabilities.
- `FocusedNoteDeletionBridge`: focused-note deletion helper.
- `NoteListModelContractBridge`: dynamic note-list search/selection contract adapter used by `ListBarLayout.qml`.
- `PanelViewModel` and `PanelViewModelRegistry`: panel-specific viewmodel routing and hook dispatch.

## Why This Layer Exists
Without these bridge objects, QML components would either:
- depend directly on large concrete viewmodels, or
- duplicate capability detection and guard logic in JavaScript.

The bridge layer keeps capability checks, ownership assumptions, and architecture-policy verification in one place.
