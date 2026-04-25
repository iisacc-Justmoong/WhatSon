# `src/app/runtime/bootstrap/WhatSonQmlLaunchSupport.cpp`

## Responsibility
Implements QML root loading for the app composition root.

## Implementation Notes
- Builds a single `lvrs::QmlRootLoadSpec` per requested root.
- Uses `lvrs::loadQmlRootObjects(...)` so initial properties and window activation follow LVRS behavior.
- Logs a warning and returns `nullptr` when LVRS reports a load failure or no root object was created.

## Test Coverage
`test/cpp/suites/app_launch_support_tests.cpp` checks that this helper uses the LVRS app-entry API and that
`main.cpp` no longer performs ad hoc `QQmlApplicationEngine::loadFromModule(...)` or manual window activation.
