# `src/app/models/editor/input/ContentsBreakBlockController.qml`

## Responsibility

Owns non-visual break block interaction logic for `ContentsBreakBlock.qml`.

The visual break row delegates tap activation and tag-management key handling to this controller.

## Contract

- Plain Backspace/Delete may request selected break-block deletion.
- Exact Command Up/Down may request document-boundary movement.
- Modified text-navigation chords are ignored so OS text-edit behavior is not consumed by the break block.
- `logicalLineLayoutEntries()` exports the divider's actual width as minimap row-width metadata so horizontal rules draw
  as visual-width rows instead of text-length approximations.

## Boundary

This controller is limited to atomic break tag management and selection activation.
