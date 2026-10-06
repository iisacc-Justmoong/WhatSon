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
  plugins. In that mode, the bootstrapper still requests Apple-bridge permissions but skips Qt runtime permission
  steps that would otherwise depend on the excluded plugin type.

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/permissions``  ( `docs/src/app/permissions/README.md` )
- Location:  `docs/src/app/permissions`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
