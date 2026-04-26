# `src/app/models/editor/format/ContentsStructuredEditorFormattingController.qml`

## Responsibility
Owns inline-format command handling for the structured note editor and translates a block-local visible-text
selection into one RAW `.wsnbody` source-range rewrite.

## Current Behavior
- The controller sits beside `ContentsStructuredDocumentFlow.qml`, not inside one text delegate.
- It resolves the active interactive block from the structured flow, queries the mounted delegate only for the live
  selection snapshot, and then performs the actual formatting rewrite itself.
- RAW rewrite authority therefore now stays at the editor-flow layer:
  - resolve the interactive block source span
  - convert the visible selection range into inline-tag-aware source edits with
    `ContentsTextFormatRenderer.applyInlineStyleToLogicalSelectionSource(...)`
  - emit one flow-level `replaceSourceRange(...)` request with source-offset-based focus restoration
- The controller works for ordinary parsed text blocks and for flattened interactive prose groups equally, because it
  always targets the currently interactive block slice rather than a parser paragraph object.
- Focus recovery stays visible-text-aware:
  - the logical cursor remains in plain-text coordinates
  - the restored RAW focus offset is recomputed through
    `ContentsStructuredCursorSupport.sourceOffsetForInlineTaggedCursor(...)`
  - the flow also carries `targetBlockIndex` so reparsed focus can prefer the same interactive row

## Architecture Note
- This file is the structured editor's authoritative inline-format mutation path.
- Block delegates may still expose selection snapshots and overlay rendering, but they no longer own shortcut-driven
  formatting writes.
