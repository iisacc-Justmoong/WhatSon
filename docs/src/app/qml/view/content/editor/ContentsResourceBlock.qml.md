# `src/app/qml/view/content/editor/ContentsResourceBlock.qml`

## Responsibility
Renders one canonical `<resource ... />` span as a normal atomic block inside structured document flow.

## Current Behavior
- This block no longer keeps the old `before / selected / after` interaction state machine or its left/right boundary
  editors.
- A click now selects the whole block, and `Backspace` / `Delete` immediately emit `blockDeletionRequested()`.
- Arrow keys emit only the generic `boundaryNavigationRequested(axis, side)` contract.
  `ContentsStructuredDocumentFlow.qml` is now responsible for deciding which neighboring block receives focus next.
- `applyFocusRequest(...)` treats any ordinary `sourceOffset` inside the resource span as whole-block selection.
  There is no dedicated `interactionMode` branching anymore.
- The block still merges parser-owned block metadata with the resolved resource entry before passing the payload into
  `ContentsResourceRenderCard.qml`.
- For gutter/current-line alignment, the block contributes one logical line and exposes
  `currentLogicalLineNumber`, `logicalLineLayoutEntries()`, and `currentCursorRowRect()`.
- The block also exports the shared atomic-block contract explicitly:
  `textEditable=false`, `atomicBlock=true`, `gutterCollapsed=true`,
  `minimapVisualKind=block`, `minimapRepresentativeCharCount=160`, and an empty `visiblePlainText()`.
- Structured shortcut insertion uses the block's trailing source offset (`focusSourceOffset`).

## Architecture Note
- This file no longer owns a special text-insertion surface before or after the attachment.
- Resource rows remain plain atomic document blocks inside the shared block stream, and surrounding document traversal
  is resolved by the flow host plus neighboring text blocks.
