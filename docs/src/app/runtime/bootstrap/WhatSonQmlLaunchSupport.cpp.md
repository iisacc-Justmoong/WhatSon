# `src/app/runtime/bootstrap/WhatSonQmlLaunchSupport.cpp`

## Responsibility
Implements QML root loading for the app composition root.

## Implementation Notes
- Builds a single `lvrs::QmlRootLoadSpec` per requested root.
- Uses `lvrs::loadQmlRootObjects(...)` so initial properties and window activation follow LVRS behavior.
- Returns the full LVRS `QmlRootLoadResult` for lifecycle-aware callers.
- Logs a warning when LVRS reports a load failure or no root object was created.
- Keeps `QObject*` wrappers only as compatibility helpers layered over `lastRootObject(...)`.

## Test Coverage
`test/cpp/suites/app_launch_support_tests.cpp` checks that this helper uses the LVRS app-entry API and that
`main.cpp` no longer performs ad hoc `QQmlApplicationEngine::loadFromModule(...)`, manual window activation, or
manual lifecycle root/window reconstruction.
