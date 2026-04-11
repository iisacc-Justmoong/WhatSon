# `src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml`

## Responsibility
Hosts the document-native block editor for structured `.wsnbody` content.

## Key Behavior
- Consumes `ContentsStructuredBlockRenderer.renderedDocumentBlocks`.
- Renders text/agenda/callout/break as one ordered document column.
- Rewrites the authoritative RAW source string on every block edit, then asks the parent view to persist the new
  source.
- Keeps a lightweight focus request channel so shortcut insertion and backend-driven Enter rules can move focus into the
  newly materialized block after reparsing.
- Preserves agenda/callout local caret positions across whole-document reparses by forwarding both the RAW source offset
  and the block-local cursor position in focus requests.
- Owns structured shortcut insertion once a document has entered block-flow mode and now asks the active delegate for
  the insertion source offset before falling back to the block end.
- Large block lists now load delegate instances asynchronously, and each late-loaded delegate immediately replays the
  latest focus request so note open does not stall on one synchronous block-instantiation burst.
