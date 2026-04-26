# `src/app/models/editor/structure`

## Responsibility
Owns the structured note document host and policies used by the editor body.

## Current Modules
- `ContentsStructuredDocumentBlocksModel.*`
  Publishes the ordered structured block stream to QML without resetting stable suffix rows.
- `ContentsStructuredDocumentCollectionPolicy.*`
  Normalizes parser/QML block collections and resource entries.
- `ContentsStructuredDocumentFocusPolicy.*`
  Computes focus restoration requests for structured blocks.
- `ContentsStructuredDocumentHost.*`
  Holds active structured-document state and publishes selection-clear revisions.
- `ContentsStructuredDocumentMutationPolicy.*`
  Builds RAW source insertion, deletion, merge, and split payloads for structured block edits.
- `ContentsLogicalLineLayoutSupport.js`
  Maps live `TextEdit` geometry into structured block logical-line entries.
- `ContentsStructuredCursorSupport.js`
  Converts plain-text cursor positions to RAW source offsets for structured agenda, callout, and semantic text blocks.

## Boundary
- These types are editor-domain structure helpers because they coordinate ordinary note body blocks.
- Parser output remains in `src/app/models/editor/parser`; HTML rendering remains in `src/app/models/editor/renderer`.
