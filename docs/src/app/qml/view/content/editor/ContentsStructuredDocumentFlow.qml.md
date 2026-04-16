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
  - `blockDeletionRequested(direction)`
  - `boundaryNavigationRequested(axis, side)`
  - `documentEndEditRequested()`
  - agenda/callout backend mutation signals
- Visible text for ordinary structured text blocks is now the same RAW block string that the text delegate edits.
  Gutter/minimap/logical-line calculations therefore no longer depend on an inline-tag-stripped plain-text projection.
- Plain newline-delimited prose now enters the flow as one ordered paragraph stream instead of one aggregated fallback
  text block.
  Blank lines below an inline resource therefore materialize as actual empty text blocks that arrow navigation and
  focus restoration can target directly.
- Those zero-length paragraph blocks are now also deletable.
  When the block has wrapper source (`<paragraph></paragraph>`), the flow removes that wrapper span; when it is an
  implicit blank line between prose blocks, the flow removes the adjacent newline token that created the empty line.
- Atomic block focus no longer branches through `interactionMode: "before" / "after"`.
  Resource and break blocks are re-entered by source offsets inside their span and resolve to whole-block selection.
- Atomic-block target focus now also carries an explicit `targetBlockIndex`.
  When adjacent blocks share the same source boundary, flow-level focus restore can still choose the resource/break
  block itself instead of letting a neighboring text block consume the same offset first.
- The flow now also accepts one generic `shortcutKeyPressHandler` from the host and forwards it into every mounted
  document block.
  Structured notes therefore can share note-wide shortcut interception, such as clipboard-image paste, without falling
  back to the legacy whole-note editor path.
- The parser now supplies the same block-trait payload before delegates finish loading, so nearest-editable-block
  fallback, logical line counting, gutter collapse, and minimap sampling no longer depend on the delegate having
  mounted already.
- The flow now keeps one cached logical-line table and cached block layout summary per parsed snapshot instead of
  rebuilding them on every gutter/minimap/current-line query.
- The block host also now virtualizes delegate loading against the bound viewport:
  off-screen text/resource/agenda/callout delegates are unloaded outside an overscanned window while their host items
  keep a cached or estimated block height so document order and source-based focus math remain stable.
- EOF resource insertion no longer restores a synthetic boundary mode; it restores focus to an offset inside the
  inserted block instead.
- The dedicated resource-local plain-text adjacent-insertion path has been removed.
  Resource blocks now participate in the same generic block stream as every other block.

## Architecture Note
- This file is not a source of truth for the structured editor state.
- Block delegates emit `nextSourceText` mutations upward, and the next visible document tree still comes only from
  reparsing that updated RAW snapshot.
