# `src/app/qml/view/content/editor/ContentsCalloutBlock.qml`

## Responsibility
Renders one callout card as a native document block inside the editor flow.

## Key Behavior
- Fills the available editor width.
- Uses a variable-height frame and full-height vertical divider, matching the multi-line callout contract.
- Edits callout body text directly inside the card instead of through a detached overlay.
- Restores the local callout caret after a RAW rewrite by converting the reparsed source offset back into the plain-text
  editor cursor position.
- Exposes `applyFocusRequest(...)` for direct targeted calls from `ContentsStructuredDocumentFlow.qml`; the block no
  longer re-evaluates every global focus token change itself.
- Programmatic focus restoration now also emits `activated()`, keeping the flow host's active-block tracking aligned
  even when the same callout editor already owned focus and only the cursor moved.
- The block now also exposes whether its nested editor currently owns focus.
  Structured-flow hosts can therefore keep idle note-refresh guards disabled while the user is still typing inside a
  callout card.
- The block now also exposes its current local logical line number based on the live callout editor cursor.
  Structured-flow hosts use that to align current-line indicators with the actual authored callout row instead of the
  card's first line only.
- While the callout editor keeps focus, cursor moves now also re-emit `activated()`.
  The host therefore refreshes current-line indicator placement when the caret moves to another authored callout line
  inside the same card.
- The callout block now also exposes `currentCursorRowRect()` in block-local coordinates.
  Structured-flow hosts use that to align current-line gutter indicators with the actual visual caret row inside
  wrapped callout text.
- Keeps agenda/callout shortcut insertion block-scoped so new proprietary wrappers are inserted after the current
  callout instead of nesting inside callout body content.
- Treats Enter on an already-empty trailing line as the "exit callout" gesture.
