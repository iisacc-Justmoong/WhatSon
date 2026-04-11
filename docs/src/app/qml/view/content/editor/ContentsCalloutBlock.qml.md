# `src/app/qml/view/content/editor/ContentsCalloutBlock.qml`

## Responsibility
Renders one callout card as a native document block inside the editor flow.

## Key Behavior
- Fills the available editor width.
- Uses a variable-height frame and full-height vertical divider, matching the multi-line callout contract.
- Edits callout body text directly inside the card instead of through a detached overlay.
- Restores the local callout caret after a RAW rewrite by converting the reparsed source offset back into the plain-text
  editor cursor position.
- Keeps agenda/callout shortcut insertion block-scoped so new proprietary wrappers are inserted after the current
  callout instead of nesting inside callout body content.
- Treats Enter on an already-empty trailing line as the "exit callout" gesture.
