# `src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml`

## Responsibility
Hosts the document-native block editor for structured `.wsnbody` content.

## Key Behavior
- Consumes `ContentsStructuredBlockRenderer.renderedDocumentBlocks`.
- Also consumes `ContentsBodyResourceRenderer.renderedResources` so `type=resource` blocks can resolve from the
  canonical `<resource ... />` source tag to the real asset file inside the `.wsresource` package.
- Renders text/agenda/callout/resource/break as one ordered document column.
- Rewrites the authoritative RAW source string on every block edit, then asks the parent view to persist the new
  source.
- Keeps a lightweight focus request channel so shortcut insertion and backend-driven Enter rules can move focus into the
  newly materialized block after reparsing.
- Preserves agenda/callout local caret positions across whole-document reparses by forwarding both the RAW source offset
  and the block-local cursor position in focus requests.
- Owns structured shortcut insertion once a document has entered block-flow mode and now asks the active delegate for
  the insertion source offset before falling back to the block end.
- Resolves each pending focus request to one target block index before dispatch:
  - agenda task focus prefers `taskOpenTagStart`
  - otherwise the host falls back to the reparsed `sourceOffset`
  - only the resolved delegate receives `applyFocusRequest(...)`
- The pending focus channel is now single-shot and tokenless:
  - `requestFocus(...)` stores one cloned request plus its current target block index
  - `documentBlocksChanged` is the only later point that re-resolves that target after reparsing
  - successful application clears the pending request instead of retaining an incrementing replay token
- Large block lists still load delegate instances asynchronously, but late-loaded delegates now replay focus only when
  they are the resolved target block instead of rebroadcasting the request through the whole block tree.
- Resource blocks resolve their render payload in three passes:
  - first by shared `resourceIndex`
  - then by matching `sourceStart/sourceEnd`
  - finally by `resourceId` or `resourcePath`
  This keeps inline resource frames stable even when the note reparse and the resource resolver refresh land on
  adjacent event-loop turns.
