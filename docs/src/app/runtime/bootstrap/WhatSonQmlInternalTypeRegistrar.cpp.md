# `src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.cpp`

## Responsibility
Registers the QObject-backed internal QML bridge types used by the application shell and editor surfaces.

## Registered Bridges
- Editor/document bridges such as `ContentsEditorSelectionBridge`, `ContentsEditorSessionController`,
  `ContentsLogicalTextBridge`, and the structured-document support types.
- Content-surface helpers from `src/app/models/content/display`, `src/app/models/content/mobile`, and
  `src/app/models/content/structured`, so runtime bootstrap resolves the real model-domain paths instead of relying on
  flattened legacy include aliases.
- Rendering/annotation helpers such as `ContentsTextFormatRenderer`, `ContentsStructuredBlockRenderer`,
  `ContentsAgendaBackend`, `ContentsCalloutBackend`, `ContentsBodyResourceRenderer`, and `ResourceBitmapViewer`.
- Workspace interaction bridges such as `FocusedNoteDeletionBridge`, `NoteListModelContractBridge`,
  `HierarchyDragDropBridge`, and `HierarchyInteractionBridge`.
- Onboarding-specific native platform bridge `WhatSonIosHubPickerBridge`, which exposes the iOS Files/Box picker to
  QML without pushing document-picker logic into `main.cpp`.

## Architectural Note
This file remains the narrow bootstrap seam for QML-visible helper types. Adding the iOS onboarding picker here keeps
the platform-native dialog integration isolated from the composition root while still allowing `OnboardingContent.qml`
to switch platforms declaratively.
