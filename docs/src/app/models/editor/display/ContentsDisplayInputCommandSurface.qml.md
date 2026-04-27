# `src/app/models/editor/display/ContentsDisplayInputCommandSurface.qml`

## Responsibility

Owns the editor display command surface for tag-management shortcuts and context-menu pointer triggers.

## Boundary

- May use `Shortcut`, pointer handlers, and `LV.ContextMenu`.
- Must not install ordinary text input handlers.
- Does not act as a ViewModel.
- Right-click selection menus are driven by a dedicated `TapHandler`, not a full-surface `MouseArea`.
  The handler primes the current editor selection on press and then asks `ContentsDisplayView.qml` to open the menu on
  tap, so a native `TextEdit` right-click cannot erase the captured selection before the menu opens.
