# `src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml`

## Responsibility
Hosts the document-native block editor for structured `.wsnbody` content.

## Key Behavior
- Consumes `ContentsStructuredBlockRenderer.renderedDocumentBlocks`.
- Also consumes `ContentsBodyResourceRenderer.renderedResources` so `type=resource` blocks can resolve from the
  canonical `<resource ... />` source tag to the real asset file inside the `.wsresource` package.
- The flow now also normalizes non-Array Qt sequence wrappers for both `renderedDocumentBlocks` and
  `renderedResources`, accepting `length`, `count`, or numeric object keys.
  Structured resource blocks therefore do not lose their resolved payload just because the backing `QVariantList`
  reaches QML through a wrapper object instead of a native JS array.
- Renders text/agenda/callout/resource/break as one ordered document column.
- The ordered document column now keeps a `10px` inter-block gap so mixed prose + image notes read like a markdown/
  HTML body rather than one collapsed zero-gap stack.
- That document column is expected to receive the host's effective note-body width, not the raw editor viewport width.
  On desktop/mobile screen editor surfaces, the host therefore subtracts its body insets before passing width into the
  structured-flow surface, keeping inline resource frames aligned with surrounding text content.
- Rewrites the authoritative RAW source string on every block edit, then asks the parent view to persist the new
  source.
- Text-block delegates now obey the same one-way edit contract as the main editor host:
  user input is interpreted as a RAW-source mutation request, the flow host replaces only the affected RAW range, and
  the next visible block tree comes only from reparsing that new RAW source.
  The host no longer treats any delegate RichText/DOM surface as a write-authoritative document snapshot.
- Keeps a lightweight focus request channel so shortcut insertion and backend-driven Enter rules can move focus into the
  newly materialized block after reparsing.
- Preserves agenda/callout local caret positions across whole-document reparses by forwarding both the RAW source offset
  and the block-local cursor position in focus requests.
- Owns structured shortcut insertion once a document has entered block-flow mode and now asks the active delegate for
  the insertion source offset before falling back to the block end.
- The same active-block insertion bridge now also accepts imported resource-tag batches from the parent host.
  A note that is already in structured-flow mode therefore inserts dropped `<resource ... />` blocks next to the
  active block instead of falling back to the legacy inline-editor cursor path or appending at EOF.
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
- When several candidates match the same block, the flow now prefers the entry that already carries a real payload path
  (`source` or `resolvedPath`) over metadata-only partial matches. This prevents one early placeholder match from
  pinning the block to a non-designed generic document summary surface after the actual asset path has resolved.
- Numeric block identity inside the flow must treat `0` as a valid value, not as "missing".
  The host therefore normalizes `resourceIndex`, focus target indices, and agenda-task tag starts with an explicit
  finite-number helper instead of `Number(value) || -1`, which previously broke the first resource block and the first
  block-focus target by collapsing index `0` to the fallback sentinel.
- The host now also exposes `requestDocumentEndEdit()` for editor-shell click-to-append behavior.
  When the document already ends in a text block, it focuses that block at its end source offset.
  When the document ends in a non-text block such as `<resource ... />` or `</break>`, it appends one trailing newline
  to materialize a new text block and then restores focus at that new end-of-document insertion point.
  If the RAW source has already received that trailing newline but the block parser has not yet caught up, the helper
  reuses the pending end offset instead of appending a second newline.
