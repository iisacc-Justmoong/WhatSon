# `src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.hpp`

## Responsibility
Declares the bootstrap helper that registers QML-internal bridge types under
`WhatSon.App.Internal`.

## Contract
- `registerInternalQmlTypes()`: one-shot registration entrypoint called by `main.cpp` after LVRS
  baseline type registration.

## Rationale
QML type registration is composition infrastructure, not runtime domain behavior.
Keeping it outside `main.cpp` reduces the composition root surface and keeps registration
responsibility isolated.
