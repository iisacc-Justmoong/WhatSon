# `src/app/qml/view/content/editor/ContentsCalloutBlock.qml`

## Responsibility
Renders one callout card as a native document block inside the editor flow.

## Key Behavior
- Non-visual callout editing state now lives in
  `src/app/models/editor/input/ContentsCalloutBlockController.qml`.
  This view keeps layout and styling, while live text snapshots, cursor geometry, selection cleanup, focus requests,
  and committed text emission belong to the controller.
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
- Callout logical-line layout now also routes through `ContentsLogicalLineLayoutSupport.js`, so wrapped callout text
  exports block-local `{contentY, contentHeight}` entries from live editor rectangles instead of one evenly divided
  block-height estimate.
  Those entries also include measured line width and wrapped visual-row widths for minimap silhouette drawing.
- The block now also exposes `clearSelection(preserveFocusedEditor)`, allowing structured-flow activation to clear
  stale callout highlight without disabling `persistentSelection` for the active editor.
- The block now also exports the shared document-block contract for flow-level layout:
  `textEditable=true`, `atomicBlock=false`, `gutterCollapsed=false`,
  `minimapVisualKind=text`, `visiblePlainText()` as the live callout editor text, and
  `representativeCharCount(...)` per visible line.
- The callout editor keeps the last live body text emitted upward and passes it back as the expected previous field text.
  Fast mobile typing/backspace therefore rebases from the live editor state rather than from an older block snapshot.
- The callout body no longer defines custom boundary-navigation key handlers.
  The nested `TextEdit` owns arrow navigation, Shift+Enter line breaks, iOS keyboard gestures, repeated delete, and
  selection behavior. Empty-callout Backspace is the only block-local delete exception and removes the whole callout
  RAW tag range.
- Focus restoration now also accepts `entryBoundary: "before" | "after"` hints from the flow host.
  Sequential block traversal can therefore enter the callout at its visual head or tail instead of always restoring to
  one generic fallback caret position.
- Keeps agenda/callout shortcut insertion block-scoped so new proprietary wrappers are inserted after the current
  callout instead of nesting inside callout body content.
- Plain Enter is the explicit callout-exit tag-management command.
  It closes the callout at the current source cursor; Shift+Enter remains the native way to insert a line break inside
  the callout body.
- Plain Backspace on an empty callout emits `blockDeletionRequested("backward")`, which the document block host forwards
  into the shared RAW block deletion path.
- The block now also accepts `paperPaletteEnabled`.
  Page/print mode therefore swaps the callout frame/divider/body text away from the dark-theme hardcoded white-text
  palette into a paper-safe light card with dark text.
