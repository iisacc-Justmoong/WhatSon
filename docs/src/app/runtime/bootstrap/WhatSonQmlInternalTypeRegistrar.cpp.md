# `src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.cpp`

## Responsibility
Builds the manifest for QObject-backed internal QML bridge types used by the application shell and editor surfaces,
then registers that manifest through LVRS `QmlTypeRegistrar`.

## Registered Bridges
- Editor/document bridges such as `ContentsEditorSelectionBridge`, `ContentsEditorSessionController`,
  `ContentsLogicalTextBridge`, and the structured-document support types.
- Editor chrome calculators and adapters such as `ContentsEditorVisualLineMetrics`, `ContentsMinimapLayoutMetrics`,
  `ContentsEditorGeometryProvider`, and `ContentsLineNumberRailMetrics`, so minimap measurement, minimap arithmetic,
  view-geometry adaptation, and line-number row construction stay in C++ model objects while QML only binds view inputs
  and resolved values.
- Content-surface helpers from `src/app/models/content/mobile` plus editor-domain helpers from
  `src/app/models/editor/display` and `src/app/models/editor/structure`, so runtime bootstrap resolves the real
  model-domain paths instead of relying on flattened legacy include aliases.
- Editor display Controllers from `src/app/models/editor/display`, including
  `ContentsEditorSurfaceModeSupport` and `ContentsEditorDisplayBackend`, so view QML can instantiate C++ command and
  surface-policy contracts instead of carrying those responsibilities locally.
  The registrar no longer exports an active-editor-surface adapter type for focus forwarding; note focus restoration
  now targets the structured document host directly from the display controllers.
- Rendering/annotation helpers such as `ContentsTextFormatRenderer`, `ContentsInlineStyleOverlayRenderer`,
  `ContentsPlainTextSourceMutator`, `ContentsStructuredBlockRenderer`, `ContentsAgendaBackend`,
  `ContentsCalloutBackend`, `ContentsBodyResourceRenderer`, and `ResourceBitmapViewer`.
- Workspace interaction bridges such as `FocusedNoteDeletionBridge`, `NoteListModelContractBridge`,
  `HierarchyDragDropBridge`, `HierarchyInteractionBridge`, and `SidebarHierarchyInteractionController`. Creatable
  controller classes registered here must remain subclassable by Qt's `QQmlElement<T>` wrapper.
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
