# `src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.cpp`

## Responsibility

Builds the manifest for QObject-backed internal QML bridge types needed by the restored application shell, then
registers that manifest through LVRS `QmlTypeRegistrar`.

## Current Contract

- The registrar no longer exports editor parser, renderer, projection, session, input-policy, minimap, line-number, tag,
  display-backend, resource-viewer, or paper helper types.
- The content QML path no longer mounts a note text editor and does not require C++ editor backend registration.

## Test Coverage

`test/cpp/suites/app_launch_support_tests.cpp` keeps this registrar on LVRS manifest registration and prevents direct
`qmlRegisterType<...>()` blocks from returning.
