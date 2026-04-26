# `src/app/qml/view/content/editor/ContentsBreakBlock.qml`

## Responsibility
Renders `</break>` as a native divider block inside the structured document flow.

## Key Behavior
- Non-visual break interaction state now lives in
  `src/app/models/editor/input/ContentsBreakBlockController.qml`.
  This view keeps only the visual divider row and delegates tap/key tag-management decisions to the controller.
- Fills the editor width.
- Keeps divider presentation in the same block stream as text, agenda, and callout nodes.
- `</break>` is now also a focusable atomic document block instead of a click-only decoration.
  The divider exposes `applyFocusRequest(...)`, block selection chrome, delete handling, and document-boundary
  navigation so it can participate in the same keyboard stream as resource blocks.
- The divider now also exports the shared atomic-block contract:
  `textEditable=false`, `atomicBlock=true`, `gutterCollapsed=true`,
  `logicalLineLayoutEntries()`, `currentCursorRowRect()`, and a minimal minimap representative length.
- Left/right/up/down navigation from the divider now delegates back into
  `ContentsStructuredDocumentFlow.qml` through one generic boundary-navigation signal.
  Break blocks therefore no longer sit outside the document cursor model.
- Modified arrow/delete chords are not consumed by the divider's host navigation layer. Plain arrow/delete remains
  atomic-block behavior, while modifier chords are left unaccepted. The divider no longer branches on `AltModifier` at
  all, so macOS Option-arrow input cannot be consumed by this atomic-block path.
- The selected divider now also runs one host-owned tag-management shortcut handler before its own delete/navigation
  rules.
  Clipboard-image paste therefore stays available even when the current structured selection is a break block rather
  than a text editor.
- A left click on the divider selects the block itself, while `documentEndEditRequested()` remains available for host
  surfaces that want to resume typing after a terminal break block.
