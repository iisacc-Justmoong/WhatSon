# `src/app/runtime/bootstrap/WhatSonQmlContextBinder.cpp`

## Responsibility
Implements root context-property binding for the workspace runtime graph.

## Bound Properties
The implementation binds all existing workspace properties used by `Main.qml` and panel/view
modules, including hierarchy viewmodels, calendar stores/viewmodels, scheduler, and panel registry.

## Test Coverage
- `tests/app/test_runtime_bootstrap_wiring.cpp` verifies that each declared property key resolves to
  the expected object pointer in `QQmlContext`.
