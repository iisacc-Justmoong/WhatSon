# `src/app/runtime/bootstrap/WhatSonQmlContextBinder.hpp`

## Responsibility
Declares workspace QML context-binding contracts used by the composition root.

## Public Contract
- `WorkspaceContextObjects`: explicit object graph payload for all root context properties consumed
  the `noteEditorSession` parsed-source bridge. It includes the restored `editorViewModeController` for navigation
  chrome, `editorInputCommandFilter` for editor item key interception, and `editorFontFamilyProvider` for the toolbar
  system-font menu.
- `bindWorkspaceContextObjects(...)`: builds an LVRS `QmlContextBindPlan`, applies it to the
  `QQmlApplicationEngine`, and returns the LVRS bind result.

## Design Intent
The composition root should assemble dependencies, not repeat long lists of context-property writes or reintroduce a
QML-side runtime registry.
