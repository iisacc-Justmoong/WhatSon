# `src/app/platform/Apple/WhatSonIosHubPickerBridge.cpp`

## Role
Provides the non-iOS stub for `WhatSonIosHubPickerBridge`.

## Behaviour
- Keeps the bridge linkable and registrable on desktop/macOS builds without pulling in UIKit.
- Returns a clear `"The native iOS hub picker is unavailable on this platform."` error from `open(...)` so accidental
  non-iOS usage fails explicitly instead of silently no-oping.
- Preserves the same `busy`/`lastError` property surface as the real iOS bridge, which keeps the QML bindings simple
  and platform-agnostic.
