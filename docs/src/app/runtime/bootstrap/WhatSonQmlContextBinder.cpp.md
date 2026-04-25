# `src/app/runtime/bootstrap/WhatSonQmlContextBinder.cpp`

## Responsibility
Implements root context and ViewModel registry binding for the workspace runtime graph through the LVRS
`QmlContextBindPlan` API.

## Bound Properties
The implementation registers the root workspace ViewModels into `LV.ViewModels` and also exposes those
objects under their stable context-property names for legacy QML consumers. Non-registry runtime services
remain context objects, including `resourcesImportViewModel`, scheduler, calendar stores/viewmodels
(`agendaViewModel`, day/week/month/year), and `panelViewModelRegistry`.

## Test Coverage

`test/cpp/suites/qml_context_binder_tests.cpp` keeps the binder on LVRS bind-plan APIs and prevents
QML-side duplicate `LV.ViewModels.set(...)` registration from returning to `Main.qml`.
