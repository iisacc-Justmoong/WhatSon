# `src/app/runtime/bootstrap/WhatSonQmlContextBinder.hpp`

## Responsibility
Declares workspace QML context-binding contracts used by the composition root.

## Public Contract
- `WorkspaceContextObjects`: explicit object graph payload for all root context properties consumed
  by QML, including Agenda/day/week/month/year calendar route controllers.
- `bindWorkspaceContextObjects(...)`: builds an LVRS `QmlContextBindPlan`, applies it to the
  `QQmlApplicationEngine`, and returns the LVRS bind result.

## Design Intent
The composition root should assemble dependencies, not repeat long lists of context-property writes or reintroduce a
QML-side runtime registry.
