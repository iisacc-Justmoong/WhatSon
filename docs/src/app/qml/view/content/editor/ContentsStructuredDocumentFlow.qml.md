# `src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml`

## Responsibility
Hosts the parsed `.wsnbody` block stream as one ordered document editor and forwards every block mutation back into RAW
source.

## Current Behavior
- Inline-format mutation authority now lives in one sibling editor controller,
  `ContentsStructuredEditorFormattingController.qml`.
  The flow exposes the same public formatting hooks to the outer host, but those hooks now delegate into a
  flow-level editor object instead of calling block-local delegate mutation methods directly.
- The flow consumes raw parsed `renderedDocumentBlocks` plus `renderedResources`, then builds a second
  interaction-only block stream before it lays the note out.
- Parsed block boundaries therefore remain available for parsing, linting, and block-local render traits, but the
  final editor `Repeater` no longer has to mirror every implicit prose paragraph one-for-one.
- Contiguous implicit text blocks are now flattened into one `text-group` interactive row:
  - the row spans the first/last child `sourceStart` / `sourceEnd`
  - the row keeps one combined `sourceText` slice from the RAW note body
  - the row mounts one shared `ContentsDocumentTextBlock.qml` editor surface for that whole prose run
- This means plain prose paragraphs remain parser-visible blocks, but final keyboard focus and caret movement no
  longer stop at every implicit paragraph boundary.
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
- Block, agenda-task, and callout text mutations now verify that the delegate's expected RAW source slice still
  matches the current document source before splicing.
  Late mobile blur/dismiss events from an older block snapshot are rejected instead of being applied to a newer source
  buffer, so an already-saved edit cannot be inserted repeatedly during note leave/session flush.
- Focused text delegates now send the expected previous live source/text with each mutation.
  The flow uses that expected slice length for text, agenda task, and callout splices, so fast iOS backspace repeat can
  continue rebasing on the latest local edit before the parser has emitted a fresh block snapshot.
- Visible text for ordinary structured text blocks is now the same RAW block string that the text delegate edits.
  Gutter/minimap/logical-line calculations therefore no longer depend on an inline-tag-stripped plain-text projection.
- Plain newline-delimited prose now enters the flow as one ordered paragraph stream instead of one aggregated fallback
  text block.
  Blank lines below an inline resource therefore materialize as actual empty text blocks that arrow navigation and
  focus restoration can target directly.
- Those parsed paragraph entries are now also flattened back into larger interactive prose rows before delegate mount.
  The parser still owns paragraph discovery, while the final editor surface owns interaction flattening.
- Those zero-length paragraph blocks are now also deletable.
  When the block has wrapper source (`<paragraph></paragraph>`), the flow removes that wrapper span; when it is an
  implicit blank line between prose blocks, the flow removes the adjacent newline token that created the empty line.
- Empty callout cards reuse the same block deletion path when their nested editor emits
  `blockDeletionRequested("backward")` from a plain Backspace key press.
- Adjacent `paragraph` / `p` blocks still share one RAW mutation-policy helper for non-native block hosts.
  The shared inline text editor no longer dispatches native text-input `Enter`, `Backspace`, or `Delete` through that
  helper, so ordinary paragraph editing stays on the OS/Qt `TextEdit` key path.
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
- Paragraph split/merge affordances are now suppressed for flattened interactive prose groups.
  Once implicit prose paragraphs have been combined into one editor span, plain `Enter` and ordinary deletion stay
  native inside that span and only cross-block transitions at the outer group edge continue to involve flow-level
  block-boundary policy.
- Atomic block focus no longer branches through `interactionMode: "before" / "after"`.
  Resource and break blocks are re-entered by source offsets inside their span and resolve to whole-block selection.
- Atomic-block target focus now also carries an explicit `targetBlockIndex`.
  When adjacent blocks share the same source boundary, flow-level focus restore can still choose the resource/break
  block itself instead of letting a neighboring text block consume the same offset first.
- The flow accepts one `tagManagementShortcutKeyPressHandler` from the host, and only selected tag-managed atomic
  blocks (`resource` and `break`) can invoke it for atomic deletion commands.
  Text-editing delegates may forward only explicit tag-management chords such as clipboard-image paste and inline
  style shortcuts from their nested `TextEdit`; native input keys, navigation, selection, text paste, and IME
  composition remain on the OS/Qt path.
- The flow exposes `nativeCompositionActive()` by scanning mounted delegates for `inputMethodComposing`/`preeditText`.
  The display host uses that state to disable window-level document shortcuts while an IME composition is active.
- Structured block navigation now also understands a document-level boundary axis in addition to adjacent
  `horizontal / vertical` hops.
  `Command + Up/Down` emitted by block delegates can therefore route straight to RAW document start/end without
  pretending that the first/last block edge is only another neighboring-block transition.
- The parser now supplies the same block-trait payload before delegates finish loading, so nearest-editable-block
  fallback, logical line counting, gutter collapse, and minimap sampling no longer depend on the delegate having
  mounted already.
- The flow now keeps one cached logical-line table and cached block layout summary per parsed snapshot instead of
  rebuilding them on every gutter/minimap/current-line query.
- The display host now also asks this flow to queue a fresh layout-cache rebuild on every note entry, even when the
  next note parses to the same logical line count as the previous one.
  Rapid note switches therefore still force one new structured geometry pass before the gutter is allowed to reuse
  note-local cached coordinates.
- Opening or remounting the structured editor now schedules a short editor-open layout-cache pass sequence.
  The first pass can still catch the immediate block stream, while the follow-up passes run after delegate creation and
  height measurement have had event-loop turns to settle.
- Block delegate load completion also queues a layout-cache refresh, so asynchronous or virtualized delegates can publish
  their measured line geometry to the gutter as soon as they mount.
- Structured cached logical-line entries now also keep `gutterContentY` aligned to each line's real `contentY`.
  Gutter-collapsed blocks still shrink only the rendered gutter box height, but line-number Y no longer comes from a
  second synthetic accumulator that ignored block spacing and delegate-local top offsets.
- Structured cached logical-line entries now also preserve measured row-width metadata (`contentWidth`,
  `contentAvailableWidth`, and `visualRowWidths`) from mounted delegates.
  The minimap can therefore cascade through the note body and draw each visible row at the width it actually occupies in
  the editor, rather than deriving the silhouette from raw character counts.
- The layout cache now also preserves any explicit per-line `gutterContentY` exported by a delegate instead of always
  overwriting it with `contentY`.
  Text-family blocks can therefore hand the flow one already block-mapped gutter origin when their inline editor's
  local origin does not match the delegate root item.
- The flow owns bottom-whitespace hit testing through `pointTargetsDocumentEndEdit(...)`.
  A click below the last rendered document block is treated as a document-end edit request, while clicks in non-block
  space above or between blocks are ignored by this route.
- Document-end focus requests now carry the last block's explicit `targetBlockIndex` with the source-end offset.
  This prevents a shared source boundary from being resolved to the previous block when the user clicks the editor's
  bottom padding to continue typing at the end.
- Resource blocks now also cap their minimap silhouette expansion to ten rows even when the inline card itself is much
  taller than ten editor lines.
  Tall images therefore stay visually present on the minimap without dominating the whole rail.
- The block host also now virtualizes delegate loading against the bound viewport:
  off-screen text/resource/agenda/callout delegates are unloaded outside an overscanned window while their host items
  keep a cached or estimated block height so document order and source-based focus math remain stable.
- The flow now also routes explicit structured selection cleanup through `ContentsStructuredDocumentHost`.
  Every block activation emits a new host selection-clear revision, and blank-area `requestDocumentEndEdit()` clears
  stale selections before it restores focus at the document tail.
- EOF resource insertion now creates a trailing editable text block and restores focus after the inserted resource tag
  instead of focusing the atomic resource card.
- Structured resource insertion now also refuses empty/no-op payloads instead of reporting success on an unchanged RAW
  snapshot.
- Structured agenda/callout/break shortcut insertion now asks `ContentsEditorBodyTagInsertionPlanner` for the canonical
  RAW tag insertion payload. The flow no longer assembles those tag strings itself; it only supplies the active
  source-offset anchor or selected source range and emits the resulting source mutation upward.
- When text is selected, callout shortcut insertion wraps that selected RAW range with explicit
  `<callout>...</callout>` tags before the parser rematerializes the visual callout block.
- Callout delegate plain-Enter exit requests now carry the source cursor into the flow, so the backend can close the
  callout at that cursor while Shift+Enter stays a native in-callout line break.
- Structured shortcut/resource insertion anchor resolution now lives in
  `ContentsStructuredDocumentHost` plus `ContentsStructuredDocumentFocusPolicy`.
  `ContentsStructuredDocumentFlow.qml` only contributes delegate-local cursor hints from the currently mounted block.
- Structured shortcut/resource insertion no longer falls back to the last block's `sourceEnd` when no interactive
  block is active, and text-editable blocks no longer guess with `sourceEnd` when they lack a live caret anchor.
  The flow now uses a live delegate cursor or pending focus `sourceOffset` and otherwise lets the caller abort or fall
  back to the outer cursor bridge.
- The dedicated resource-local plain-text adjacent-insertion path has been removed.
  Resource blocks now participate in the same generic block stream as every other block.
- An empty selected RAW note body now still produces one fallback interactive `text-group` row.
  The structured document host therefore stays focusable/editable for empty notes instead of collapsing to a blank
  center surface until the first mutation lands.

## Architecture Note
- This file is not a source of truth for the structured editor state.
- Block delegates emit `nextSourceText` mutations upward, and the next visible document tree still comes only from
  reparsing that updated RAW snapshot.
- Inline-style shortcuts therefore no longer depend on per-block delegate mutation helpers.
  They now resolve selection state from the active delegate but commit the RAW rewrite through the flow-level
  formatting controller.
