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
- Owns structured shortcut insertion once a document has entered block-flow mode.
