# `src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml`

## Responsibility
Hosts the parsed `.wsnbody` block stream as one ordered document editor and forwards every block mutation back into RAW
 source.

## Current Behavior
- The flow consumes `renderedDocumentBlocks` and `renderedResources` and lays them out as one ordered block column.
- The repeater host no longer chooses one delegate per block type directly.
  Every row now mounts `ContentsDocumentBlock.qml`, and that adapter chooses the concrete block implementation
  internally.
- The host therefore no longer owns block-type-specific wiring for `agenda / callout / resource / break / text`.
  It now consumes one shared document-block contract instead.
- Gutter/minimap/current-line/focus recovery now resolve through that shared contract rather than through direct
  `type === ...` checks:
  - `textEditable`
  - `atomicBlock`
  - `gutterCollapsed`
  - `plainText`
  - `logicalLineCountHint`
  - `minimapVisualKind`
  - `minimapRepresentativeCharCount`
- The common contracts handled at flow level are:
  - `sourceMutationRequested(...)`
  - `blockDeletionRequested()`
  - `boundaryNavigationRequested(axis, side)`
  - `documentEndEditRequested()`
  - agenda/callout backend mutation signals
- Visible text for ordinary structured text blocks is now the same RAW block string that the text delegate edits.
  Gutter/minimap/logical-line calculations therefore no longer depend on an inline-tag-stripped plain-text projection.
- Atomic block focus no longer branches through `interactionMode: "before" / "after"`.
  Resource and break blocks are re-entered by source offsets inside their span and resolve to whole-block selection.
- The parser now supplies the same block-trait payload before delegates finish loading, so nearest-editable-block
  fallback, logical line counting, gutter collapse, and minimap sampling no longer depend on the delegate having
  mounted already.
- EOF resource insertion no longer restores a synthetic boundary mode; it restores focus to an offset inside the
  inserted block instead.
- The dedicated resource-local plain-text adjacent-insertion path has been removed.
  Resource blocks now participate in the same generic block stream as every other block.

## Architecture Note
- This file is not a source of truth for the structured editor state.
- Block delegates emit `nextSourceText` mutations upward, and the next visible document tree still comes only from
  reparsing that updated RAW snapshot.
