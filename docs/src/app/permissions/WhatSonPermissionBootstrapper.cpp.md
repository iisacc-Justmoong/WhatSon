# `src/app/permissions/WhatSonPermissionBootstrapper.cpp`

## Responsibility
Implements startup-time permission request sequencing and persisted decision tracking.

## Key Behaviors
- Loads and persists per-permission decisions via `QSettings`:
  - `permissions/<permissionId>/requested`
  - `permissions/<permissionId>/granted`
- Skips already-decided permissions during subsequent boots.
- Executes each permission asynchronously and continues with `QTimer::singleShot(0, ...)`.
- Computes and stores final aggregate grant state to `permissions/granted`.

## Platform Branches
- macOS/iOS-specific Apple permission requests are routed through `WhatSon::Permissions::*` bridge functions.
- Qt runtime permissions are requested only when `QT_CONFIG(permissions)` is enabled.

## Startup Integration
- Constructed in `main.cpp`.
- Triggered from foreground service startup path through `start()`.

## Regression Checklist
- Existing granted/denied decisions must be respected without duplicate prompts.
- New undecided permission steps must run in defined order.
- Final `permissions/granted` must reflect all configured steps.
