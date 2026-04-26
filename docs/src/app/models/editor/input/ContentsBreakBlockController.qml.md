# `src/app/models/editor/input/ContentsBreakBlockController.qml`

## Responsibility

Owns non-visual break block interaction logic for `ContentsBreakBlock.qml`.

The visual break row delegates tap activation and tag-management key handling to this controller.

## Contract

- Plain Backspace/Delete may request selected break-block deletion.
- Exact Command Up/Down may request document-boundary movement.
- Option/Alt modified arrows are ignored so OS text-navigation behavior is not consumed by the break block.

## Boundary

This controller is limited to atomic break tag management and selection activation.
