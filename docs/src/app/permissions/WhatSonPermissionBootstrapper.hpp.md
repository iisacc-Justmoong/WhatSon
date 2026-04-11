# `src/app/permissions/WhatSonPermissionBootstrapper.hpp`

## Responsibility
Declares the startup permission bootstrap coordinator used by app launch wiring.

## Public Surface
- `WhatSonPermissionBootstrapper(QCoreApplication& app)`: binds to the app instance and prebuilds request steps.
- `start()`: starts sequential permission processing.

## Internal Contract
- Stores permission steps as `PermissionStep{id, request}` and runs them in-order.
- Persists request/grant decisions in `QSettings` under `permissions/<id>/*`.
- Supports both:
  - Qt permission API flow (`addQtPermissionStep(...)`)
  - Apple bridge flow (`addApplePermissionStep(...)`)

## Ordering Policy
- Build sequence includes (platform-dependent):
  - full disk access
  - photo library
  - microphone
  - accessibility
  - calendar
  - reminders
  - local network
  - location

## Completion Rule
- After all steps, writes aggregated grant state to `permissions/granted`.
