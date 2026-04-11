# `src/app/qml/view/content/editor/ContentsCalloutBlock.qml`

## Responsibility
Renders one callout card as a native document block inside the editor flow.

## Key Behavior
- Fills the available editor width.
- Uses a variable-height frame and full-height vertical divider, matching the multi-line callout contract.
- Edits callout body text directly inside the card instead of through a detached overlay.
- Treats Enter on an already-empty trailing line as the "exit callout" gesture.
