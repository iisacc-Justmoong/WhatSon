# `src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.hpp`

## Responsibility
Declares the bootstrap helper that registers QML-internal bridge types under
`WhatSon.App.Internal`.

## Contract
- `internalQmlTypeRegistrationManifest()`: declares all `WhatSon.App.Internal` type registrations as an LVRS manifest.
- `registerInternalQmlTypes()`: one-shot registration entrypoint called by `main.cpp` after LVRS baseline type
  registration; returns an LVRS registration report.

## Rationale
QML type registration is composition infrastructure, not runtime domain behavior. Keeping it outside `main.cpp`
reduces the composition root surface, while the LVRS manifest result lets startup fail with a concrete diagnostic
if an internal bridge type cannot be registered.
