# `src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml`

## Responsibility
Hosts the parsed `.wsnbody` block stream as one ordered document editor and forwards every block mutation back into RAW
source.

## Current Behavior
- The flow consumes `renderedDocumentBlocks` and `renderedResources` and lays them out as one ordered block column.
- Parsed `renderedDocumentBlocks` snapshots no longer bind straight into the `Repeater` as ephemeral JS arrays.
  The flow now routes them through `ContentsStructuredDocumentBlocksModel`, which preserves retained rows and emits
  incremental row/data updates for suffix blocks whose source offsets shifted after an upstream edit.
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
  - `paragraphSplitRequested(sourceOffset)`
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
- Adjacent `paragraph` / `p` blocks are now also mergeable and splittable through one shared RAW mutation-policy path.
  `Backspace` at the start of the later paragraph merges into the previous paragraph, `Delete` at the end merges the
  next paragraph into the current one, and plain `Enter` splits the paragraph at the visible caret.
- Paragraph-boundary operations stay limited to paragraph-style prose blocks only.
  The flow consults `ContentsStructuredDocumentMutationPolicy` before exposing merge/split affordances, so headings,
  framed blocks, and atomic resources do not accidentally participate in paragraph joins.
- Explicit paragraph wrappers are preserved by policy rather than by per-delegate string surgery.
  Splitting `<paragraph>foo</paragraph>` now produces two explicit wrappers, while merging explicit and implicit
  paragraphs keeps the earlier block as the surviving RAW authority.
- Adjacent text-family blocks no longer inherit one global column gap.
  Structured prose-like tags such as `paragraph`, `title`, `subTitle`, `eventTitle`, and the other text delegates now
  keep zero extra inter-block spacing, while framed blocks such as `resource`, `break`, `agenda`, and `callout`
  still reserve their visual separation from neighboring prose.
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
- The display host now also asks this flow to queue a fresh layout-cache rebuild on every note entry, even when the
  next note parses to the same logical line count as the previous one.
  Rapid note switches therefore still force one new structured geometry pass before the gutter is allowed to reuse
  note-local cached coordinates.
- Structured cached logical-line entries now also keep `gutterContentY` aligned to each line's real `contentY`.
  Gutter-collapsed blocks still shrink only the rendered gutter box height, but line-number Y no longer comes from a
  second synthetic accumulator that ignored block spacing and delegate-local top offsets.
- The layout cache now also preserves any explicit per-line `gutterContentY` exported by a delegate instead of always
  overwriting it with `contentY`.
  Text-family blocks can therefore hand the flow one already block-mapped gutter origin when their inline editor's
  local origin does not match the delegate root item.
- Resource blocks now also cap their minimap silhouette expansion to ten rows even when the inline card itself is much
  taller than ten editor lines.
  Tall images therefore stay visually present on the minimap without dominating the whole rail.
- The block host also now virtualizes delegate loading against the bound viewport:
  off-screen text/resource/agenda/callout delegates are unloaded outside an overscanned window while their host items
  keep a cached or estimated block height so document order and source-based focus math remain stable.
- The flow now also routes explicit structured selection cleanup through `ContentsStructuredDocumentHost`.
  Every block activation emits a new host selection-clear revision, and blank-area `requestDocumentEndEdit()` clears
  stale selections before it restores focus at the document tail.
- EOF resource insertion no longer restores a synthetic boundary mode; it restores focus to an offset inside the
  inserted block instead.
- Structured resource insertion now also refuses empty/no-op payloads instead of reporting success on an unchanged RAW
  snapshot.
- Structured shortcut/resource insertion anchor resolution now lives in
  `ContentsStructuredDocumentHost` plus `ContentsStructuredDocumentFocusPolicy`.
  `ContentsStructuredDocumentFlow.qml` only contributes delegate-local cursor hints from the currently mounted block.
- Structured shortcut/resource insertion no longer falls back to the last block's `sourceEnd` when no interactive
  block is active, and text-editable blocks no longer guess with `sourceEnd` when they lack a live caret anchor.
  The flow now uses a live delegate cursor or pending focus `sourceOffset` and otherwise lets the caller abort or fall
  back to the outer cursor bridge.
- The dedicated resource-local plain-text adjacent-insertion path has been removed.
  Resource blocks now participate in the same generic block stream as every other block.

## Architecture Note
- This file is not a source of truth for the structured editor state.
- Block delegates emit `nextSourceText` mutations upward, and the next visible document tree still comes only from
  reparsing that updated RAW snapshot.
