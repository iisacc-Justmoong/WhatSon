# `src/app/runtime/bootstrap/WhatSonQmlContextBinder.cpp`

## Responsibility
Implements root context-object binding for the workspace runtime graph through the LVRS
`QmlContextBindPlan` API.

## Bound Properties
The implementation registers the root workspace objects under stable context-property names. This includes hierarchy,
detail, navigation, import, scheduler, calendar, panel-runtime controllers, the app-wide `noteActiveState` tracker, and
the `noteEditorSession` parsed-source bridge. It also registers `editorViewModeController` so navigation chrome can
render the restored `Plain/Page/Print/Web/Presentation` combo while the workspace editor surface remains the LVRS
`TextEditor` path. `editorInputCommandFilter` is bound beside clipboard and note session objects so QML can attach one
C++ filter to the public LVRS editor item. `editorFontFamilyProvider` is bound beside those editor command objects so
the toolbar can render a provider-backed system-font context menu without QML querying platform font APIs.

## Test Coverage

`test/cpp/suites/qml_context_binder_tests.cpp` keeps the binder on LVRS bind-plan APIs and prevents registry-based
controller or view-model binding from returning to `Main.qml`.
