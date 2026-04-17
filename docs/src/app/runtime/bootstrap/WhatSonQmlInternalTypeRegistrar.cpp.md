# `src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.cpp`

## Responsibility
Provides the concrete QML bridge registration list used by the workspace shell.

## Registered Bridges
- `ContentsEditorSelectionBridge`
- `ContentsLogicalTextBridge`
- `ContentsGutterMarkerBridge`
- `ContentsResourceTagTextGenerator`
- `ContentsTextFormatRenderer`
- `ContentsStructuredBlockRenderer`
- `ContentsStructuredTagValidator`
- `ContentsAgendaBackend`
- `ContentsCalloutBackend`
- `ContentsPagePrintLayoutRenderer`
- `ContentsBodyResourceRenderer`
- `ResourceBitmapViewer`
- `FocusedNoteDeletionBridge`
- `NoteListModelContractBridge`
- `HierarchyDragDropBridge`
- `HierarchyInteractionBridge`

## Architectural Note
This file is intentionally a narrow bootstrap adapter so changes to bridge registration do not
inflate the composition root in `main.cpp`.

Qt registers QObject-based bridges here through `qmlRegisterType(...)`, so registered bridge types must remain
compatible with Qt's internal `QQmlElement<T>` wrapper (for example, do not declare those bridge classes as `final`).
