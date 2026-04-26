# `src/app/models/editor/input/ContentsDocumentBlockController.qml`

## Responsibility

Owns non-visual behavior for the generic `ContentsDocumentBlock.qml` adapter.

It forwards mounted delegate signals, resolves generic block queries, handles atomic-block focus/selection, and keeps
tag-management key handling out of the visual adapter shell.

## Contract

- Text delegates keep ordinary input on their nested `TextEdit`.
- Atomic resource and break blocks may consume plain selection/delete keys and exact Command Up/Down document-boundary
  commands.
- Modified text-navigation chords are not consumed by this controller, preserving OS text-edit behavior.
- Mounted delegate signals are re-emitted through the outer document-block contract.

## Boundary

The controller may only own tag-management and atomic-block selection behavior. It must not become a shared text
boundary or markdown shortcut key handler.
