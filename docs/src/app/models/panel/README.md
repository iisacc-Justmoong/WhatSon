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
- `NoteActiveStateTracker`: app-wide active-note state tracker that follows the active hierarchy context, publishes
  normalized `activeNoteId` / `activeNoteDirectoryPath` / `activeNoteEntry` for QML. It stops at selection publication
  and does not mount editor sessions, mutate note source, or participate in editor persistence.
- `NoteListModelContractBridge`: dynamic note-list search/selection contract adapter used by `ListBarLayout.qml`.
- `PanelController` and `PanelControllerRegistry`: panel-specific controller routing and hook dispatch.

## Why This Layer Exists
Without these bridge objects, QML components would either:
- depend directly on large concrete controllers, or
- duplicate capability detection and guard logic in JavaScript.

The bridge layer keeps capability checks, ownership assumptions, and architecture-policy verification in one place.

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/models/panel``  ( `docs/src/app/models/panel/README.md` )
- Location:  `docs/src/app/models/panel`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
