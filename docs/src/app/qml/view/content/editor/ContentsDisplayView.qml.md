# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility
Desktop content editor host.

## Structured Document Flow
- Structured notes now switch from the legacy overlay composition into `ContentsStructuredDocumentFlow.qml`.
- The flow renderer is activated whenever `ContentsStructuredBlockRenderer` reports a non-text block
  (`agenda`, `callout`, or `break`).
- In that mode, agenda/callout cards are rendered as document-owned blocks inside the scroll flow instead of being
  painted above/below a single `TextEdit`.
- Desktop structured shortcuts (`Cmd+Opt+T`, `Cmd+Opt+C`, `Cmd+Shift+H`) now dispatch through the view so they can
  target either the legacy single-editor path or the new structured flow path.
- Source persistence for block edits now runs through `applyDocumentSourceMutation(...)`, which updates the RAW body,
  refreshes the presentation snapshot, marks local authority, and optionally restores focus inside the reparsed block.

## Legacy Surface
- The single `ContentsInlineFormatEditor` and overlay layers still exist as the fallback path for notes without any
  structured blocks.
