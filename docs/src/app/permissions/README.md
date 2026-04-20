# `src/app/permissions`

## Responsibility
Hosts application permission integration code:
- platform permission bridges (`ApplePermissionBridge`)
- startup-time permission sequencing (`WhatSonPermissionBootstrapper`)

## Scope
- Source directory: `src/app/permissions`
- Child directories: none
- Child files: 5

## Child Files
- `ApplePermissionBridge.hpp`
- `ApplePermissionBridge.mm`
- `ApplePermissionBridge_stub.cpp`
- `WhatSonPermissionBootstrapper.hpp`
- `WhatSonPermissionBootstrapper.cpp`

## Architectural Notes
- `WhatSonPermissionBootstrapper` was consolidated from `src/app/runtime/permissions` into this domain so permission
  request policy and platform bridge implementations stay in one module boundary.
- Runtime startup orchestrators consume this module rather than owning permission policy directly.

## Dependency Direction
- Consumed by `main.cpp` startup wiring.
- Depends on Qt permission APIs (`QPermission`, `QMicrophonePermission`, `QCalendarPermission`, `QLocationPermission`)
  and Apple bridge request functions.
- iOS simulator-safe exports may define `WHATSON_DISABLE_QT_PERMISSION_REQUESTS=1` from
  `src/app/cmake/runtime/CMakeLists.txt` when `WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY` excludes Qt `permissions`
  plugins. In that mode, the bootstrapper still requests Apple-bridge permissions but skips Qt runtime permission
  steps that would otherwise depend on the excluded plugin type.
