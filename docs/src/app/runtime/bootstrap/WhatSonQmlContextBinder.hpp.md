# `src/app/runtime/bootstrap/WhatSonQmlContextBinder.hpp`

## Responsibility
Declares workspace QML context-binding contracts used by the composition root.

## Public Contract
- `WorkspaceContextObjects`: explicit object graph payload for all root context properties consumed
  by QML.
- `bindWorkspaceContextObjects(...)`: writes the object graph into `QQmlContext`.

## Design Intent
The composition root should assemble dependencies, not repeat long lists of context-property write
calls inline.
