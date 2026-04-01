# `src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.cpp`

## Responsibility
Provides the concrete QML bridge registration list used by the workspace shell.

## Registered Bridges
- `ContentsEditorSelectionBridge`
- `ContentsLogicalTextBridge`
- `ContentsGutterMarkerBridge`
- `ContentsBodyResourceRenderer`
- `FocusedNoteDeletionBridge`
- `NoteListModelContractBridge`
- `HierarchyDragDropBridge`
- `HierarchyInteractionBridge`

## Architectural Note
This file is intentionally a narrow bootstrap adapter so changes to bridge registration do not
inflate the composition root in `main.cpp`.
