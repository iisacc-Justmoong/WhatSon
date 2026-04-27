# `src/app/qml/view/content/editor/ContentsResourceBlock.qml`

## Responsibility
Renders one canonical `<resource ... />` span as presentation-only document content inside structured document flow.

## Current Behavior
- This block no longer owns focus, key handling, delete handling, or boundary-navigation logic.
  It is now a presentation-only item.
- Whole-block selection, arrow-key traversal, delete handling, and Enter behavior are handled by
  `ContentsDocumentBlock.qml`, which uses the resource span only as one atomic document block surface.
- The block still merges parser-owned block metadata with the resolved resource entry before passing the payload into
  `ContentsResourceRenderCard.qml`.
- For gutter/current-line alignment, the block contributes one logical line and exposes
  `currentLogicalLineNumber`, `logicalLineLayoutEntries()`, and `currentCursorRowRect()`.
  Its logical-line layout entry now includes the rendered card width so the minimap paints the resource row as a visual
  silhouette instead of a character-count approximation.
- The block also exports the shared atomic-block contract explicitly:
  `textEditable=false`, `atomicBlock=true`, `gutterCollapsed=true`,
  `minimapVisualKind=block`, `minimapRepresentativeCharCount=160`, and an empty `visiblePlainText()`.
- Structured shortcut insertion uses the block's trailing source offset (`sourceEnd`), so pasting another resource while
  a resource card is focused appends after the current card instead of reusing the card's internal focus anchor.

## Architecture Note
- This file no longer behaves like an editor widget.
- Resource rows remain plain atomic document blocks inside the shared block stream, and surrounding document traversal
  is resolved by the adapter/flow layers rather than by a resource-local focus state machine.
