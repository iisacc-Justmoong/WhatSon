# `src/app/models/editor/display/ContentsDisplayInputCommandSurface.qml`

## Responsibility

Owns the editor display command surface for tag-management shortcuts and context-menu pointer triggers.

## Boundary

- May use `Shortcut`, pointer handlers, and `LV.ContextMenu`.
- Must not install ordinary text input handlers.
- Does not act as a ViewModel.
