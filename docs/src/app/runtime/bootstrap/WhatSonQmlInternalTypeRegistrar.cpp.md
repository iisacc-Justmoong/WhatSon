# `src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.cpp`

## Responsibility

Builds the manifest for QObject-backed internal QML bridge types needed by the restored application shell, then
registers that manifest through LVRS `QmlTypeRegistrar`.

## Current Contract

- The registrar no longer exports editor parser, renderer, projection, session, input-policy, minimap, line-number, tag,
  display-backend, resource-viewer, or paper helper types.
- The note editor QML path mounts `LV.TextEditor` through `src/app/qml/view/contents/TextEditor.qml`; it does not require a C++
  editor backend registration.
- Remaining registrations are shell/navigation helpers such as mobile route coordinators, note-list and hierarchy
  interaction bridges, and the iOS onboarding picker.

## Test Coverage

`test/cpp/suites/app_launch_support_tests.cpp` keeps this registrar on LVRS manifest registration and prevents direct
`qmlRegisterType<...>()` blocks from returning.
