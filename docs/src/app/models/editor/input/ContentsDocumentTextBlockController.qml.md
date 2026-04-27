# `src/app/models/editor/input/ContentsDocumentTextBlockController.qml`

## Responsibility

Owns non-visual structured text-block editing logic for `ContentsDocumentTextBlock.qml`.

The controller maps committed text edits into RAW block-source mutations, maintains focused live-edit snapshots,
computes cursor row geometry, and applies focus requests.

## Contract

- RAW `.wsnbody` block source remains authoritative.
- Plain text blocks take the direct path: the committed `TextEdit` plain text becomes the next RAW block source
  immediately, then the structured parser/renderer rematerializes the block projection from that RAW snapshot.
- Inline style tags are rendered as a read-side overlay by the view; this controller only computes the source mutation
  request produced by committed plain-text edits.
- Blocks that already contain inline style tags keep the tag-preserving path through
  `ContentsTextFormatRenderer.applyPlainTextReplacementToSource(...)` so visible plain-text edits do not erase
  `<bold>`, `<italic>`, `<highlight>`, or related style tags.
- Text-edit mutation focus requests use `reason: "text-edit"` so native-priority input sessions are not replayed over
  an active editor.
- The view exposes the block surface and delegates typing, cursor, and selection bookkeeping here.

## Boundary

This controller must not install ordinary key overrides. It reacts to committed `TextEdit` state and emits source
mutation requests upward.
