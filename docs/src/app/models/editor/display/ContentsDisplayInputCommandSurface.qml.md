# `src/app/models/editor/display/ContentsDisplayInputCommandSurface.qml`

## Responsibility

Owns the editor display command surface for tag-management shortcuts and context-menu pointer triggers.

## Boundary

- May use `Shortcut`, pointer handlers, and `LV.ContextMenu`.
- Must not install ordinary text input handlers.
- Does not act as a ViewModel.
- Desktop right-click selection menus are driven by a dedicated full-surface `MouseArea` that accepts only
  `Qt.RightButton`.
  The surface primes the current editor selection on press and then asks `ContentsDisplayView.qml` to open the menu on
  click, so native left-button text selection remains owned by the live `TextEdit` path.
- Mobile long-press still uses a separate `TapHandler`.
  Desktop right-click and mobile long-press stay split so the desktop menu path does not depend on passive pointer
  grabs inside nested editor delegates.
- On iOS the command-surface long-press handler stands down entirely, because native `TextEdit` long-press, double-tap,
  and triple-tap selection gestures must remain platform-owned instead of being intercepted by the host context-menu
  layer.
