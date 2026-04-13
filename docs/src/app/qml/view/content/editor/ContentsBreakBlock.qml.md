# `src/app/qml/view/content/editor/ContentsBreakBlock.qml`

## Responsibility
Renders `</break>` as a native divider block inside the structured document flow.

## Key Behavior
- Fills the editor width.
- Keeps divider presentation in the same block stream as text, agenda, and callout nodes.
- A left click on the divider now emits `documentEndEditRequested()`, so users can resume typing after a terminal
  break block without needing an already-existing trailing text paragraph.
