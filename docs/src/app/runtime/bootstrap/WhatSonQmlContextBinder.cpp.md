# `src/app/runtime/bootstrap/WhatSonQmlContextBinder.cpp`

## Responsibility
Implements root context-object binding for the workspace runtime graph through the LVRS
`QmlContextBindPlan` API.

## Bound Properties
The implementation registers the root workspace objects under stable context-property names. This includes hierarchy,
detail, navigation, import, scheduler, calendar, panel-runtime controllers, and the app-wide `noteActiveState` tracker.

## Test Coverage

`test/cpp/suites/qml_context_binder_tests.cpp` keeps the binder on LVRS bind-plan APIs and prevents registry-based
controller or view-model binding from returning to `Main.qml`.
