# `src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.cpp`

## Responsibility
Builds the manifest for QObject-backed internal QML bridge types used by the application shell and editor surfaces,
then registers that manifest through LVRS `QmlTypeRegistrar`.

## Registered Bridges
- Editor/document bridges such as `ContentsEditorSelectionBridge`, `ContentsEditorSessionController`,
  `ContentsLogicalTextBridge`, and the structured-document support types.
- Content-surface helpers from `src/app/models/content/mobile` plus editor-domain helpers from
  `src/app/models/editor/display` and `src/app/models/editor/structure`, so runtime bootstrap resolves the real
  model-domain paths instead of relying on flattened legacy include aliases.
- Rendering/annotation helpers such as `ContentsTextFormatRenderer`, `ContentsStructuredBlockRenderer`,
  `ContentsAgendaBackend`, `ContentsCalloutBackend`, `ContentsBodyResourceRenderer`, and `ResourceBitmapViewer`.
- Workspace interaction bridges such as `FocusedNoteDeletionBridge`, `NoteListModelContractBridge`,
  `HierarchyDragDropBridge`, and `HierarchyInteractionBridge`.
- Onboarding-specific native platform bridge `WhatSonIosHubPickerBridge`, which exposes the iOS Files/Box picker to
  QML without pushing document-picker logic into `main.cpp`.

## Architectural Note
This file remains the narrow bootstrap seam for QML-visible helper types. The manifest keeps type order, module URI,
version, and diagnostics in one place while LVRS owns validation and registration reporting. Adding the iOS onboarding
picker here keeps the platform-native dialog integration isolated from the composition root while still allowing
`OnboardingContent.qml` to switch platforms declaratively.

## Test Coverage
`test/cpp/suites/qml_internal_type_registrar_tests.cpp` keeps this registrar on LVRS manifest registration and prevents
direct `qmlRegisterType<...>()` blocks from returning.
