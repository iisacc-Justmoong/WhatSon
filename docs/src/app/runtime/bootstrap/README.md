# `src/app/runtime/bootstrap`

## Role
This directory contains bootstrap-time composition helpers extracted from `main.cpp`.

The module keeps `main.cpp` focused on orchestration flow while moving repetitive wiring blocks into
dedicated single-purpose helpers.

## Files
- `WhatSonAppLaunchSupport`: parses launcher flags and classifies whether startup can enter the workspace immediately.
- `WhatSonQmlInternalTypeRegistrar`: registers QML-instantiated internal bridge types.
- `WhatSonHubSyncWiring`: wires local mutation signals into `WhatSonHubSyncController`.
- `WhatSonQmlContextBinder`: binds workspace runtime objects into QML context properties.
