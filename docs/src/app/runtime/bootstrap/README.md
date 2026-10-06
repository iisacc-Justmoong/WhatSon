# `src/app/runtime/bootstrap`

## Role
This directory contains bootstrap-time composition helpers extracted from `main.cpp`.

The module keeps `main.cpp` focused on orchestration flow while moving repetitive wiring blocks into
dedicated single-purpose helpers.

## Files
- `WhatSonAppLaunchSupport`: parses launcher flags and classifies whether startup can enter the workspace immediately.
- `WhatSonQmlLaunchSupport`: routes QML root creation and window activation through LVRS app-entry helpers, preserving
  `QmlRootLoadResult` for lifecycle and foreground-service bootstrap.
- `WhatSonQmlInternalTypeRegistrar`: registers the remaining shell-level internal QML bridge types through an LVRS
  manifest.
- `WhatSonHubSyncWiring`: wires local mutation signals into `WhatSonHubSyncController`.
- `WhatSonQmlContextBinder`: applies the workspace LVRS context/Controller bind plan before root QML load.

`main.cpp` retains foreground-service startup ownership because scheduler and permission requests are composition-root
policy. That startup is guarded by LVRS `ForegroundServiceGate` after `WhatSonQmlLaunchSupport` has produced and
activated a visible workspace root.

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/runtime/bootstrap``  ( `docs/src/app/runtime/bootstrap/README.md` )
- Location:  `docs/src/app/runtime/bootstrap`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
- Runtime: `WhatSonQmlLaunchSupport` preserves LVRS `QmlRootLoadResult` and delivers the same root/window results to the lifecycle/foreground-service bootstrap.
