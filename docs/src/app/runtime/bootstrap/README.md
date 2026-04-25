# `src/app/runtime/bootstrap`

## Role
This directory contains bootstrap-time composition helpers extracted from `main.cpp`.

The module keeps `main.cpp` focused on orchestration flow while moving repetitive wiring blocks into
dedicated single-purpose helpers.

## Files
- `WhatSonAppLaunchSupport`: parses launcher flags and classifies whether startup can enter the workspace immediately.
- `WhatSonQmlLaunchSupport`: routes QML root creation and window activation through LVRS app-entry helpers.
- `WhatSonQmlInternalTypeRegistrar`: registers QML-instantiated internal bridge types through an LVRS manifest.
- `WhatSonHubSyncWiring`: wires local mutation signals into `WhatSonHubSyncController`.
- `WhatSonQmlContextBinder`: applies the workspace LVRS context/ViewModel bind plan before root QML load.

`main.cpp` retains foreground-service startup ownership because scheduler and permission requests are composition-root
policy. That startup is guarded by LVRS `ForegroundServiceGate` after `WhatSonQmlLaunchSupport` has produced and
activated a visible workspace root.
