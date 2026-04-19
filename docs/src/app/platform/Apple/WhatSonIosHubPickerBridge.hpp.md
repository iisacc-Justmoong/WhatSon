# `src/app/platform/Apple/WhatSonIosHubPickerBridge.hpp`

## Role
Declares the QML-facing bridge that opens the native iOS hub picker used by onboarding.

## QML Contract
- Exposes `busy` so onboarding actions can be disabled while the native picker is presented.
- Exposes `lastError` so native-picker failures can flow into the same status label used by the onboarding controller.
- Provides `open(const QUrl& initialDirectoryUrl)` for the QML surface to launch the picker near the previously used
  folder when possible.
- Emits `accepted(const QUrl& selectedUrl)` and `canceled()` so QML can either continue hub resolution or leave the
  session unchanged.

## Separation
- The header is platform-neutral and always available to the QML type registrar.
- iOS behaviour lives in the Objective-C++ implementation, while non-iOS builds receive a stub implementation that
  reports the picker as unavailable.
