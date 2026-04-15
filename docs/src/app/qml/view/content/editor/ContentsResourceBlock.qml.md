# `src/app/qml/view/content/editor/ContentsResourceBlock.qml`

## Responsibility
Renders one structured document-flow block for a canonical `<resource ... />` source tag.

## Key Behavior
- Receives the parser-owned source span from `ContentsStructuredBlockRenderer`.
- Receives the resolved asset entry from `ContentsBodyResourceRenderer` when that resolver can map the
  `.wsresource` package reference to the real payload file inside the package bundle.
- Falls back to a metadata-only `ContentsResourceRenderCard` payload when the asset has not resolved yet, so the body
  still keeps a visible block slot instead of collapsing back into plain text.
- The block now merges parser-owned block metadata (`type`, `format`, `resourcePath`, source span) into the matched
  resource entry before handing it to `ContentsResourceRenderCard.qml`.
  A matched entry is considered truly resolved only when it carries `source` or `resolvedPath`; `renderMode` alone no
  longer suppresses the block's fallback metadata.
  This keeps the first inline image block from degrading into a non-designed generic document summary surface when a
  partial renderer entry arrives before the fully resolved payload.
- Implements the same `applyFocusRequest(...)` and `shortcutInsertionSourceOffset()` contract as other structured
  delegates, which lets the document-flow host keep block focus and shortcut insertion deterministic across reparses.
- `applyFocusRequest(...)` now defaults any ordinary `sourceOffset` hit inside the resource span to whole-block
  selection.
  The left/right edge caret anchors open only when a caller explicitly requests
  `interactionMode: "before"` or `interactionMode: "after"`, so routine focus recovery no longer degrades into one
  accidental edge caret.
- The block now treats the resource row itself as one Apple Notes-like three-state token:
  - left caret lane: the `before` insertion point for the `<resource ... />` RAW span
  - center/default click area: whole-block selection of the resource token itself
  - right caret lane: the `after` insertion point for the same RAW span
  Clicking the left or right lane no longer jumps directly into surrounding prose.
  It first enters the resource token's own boundary caret state, so the attachment behaves like one document atom with
  two cursor positions plus one selected state.
- Those transient boundary editors now live in dedicated left/right caret lanes beside the frame instead of above or
  below it.
  They still feed committed plain text back into RAW through
  `adjacentPlainTextInsertionRequested(side, text, cursorPosition)`, which means typing from the left lane inserts a
  new paragraph before the `<resource ... />` tag and typing from the right lane inserts after it.
- The resource frame itself now spans the full available document-column width.
  The left/right caret lanes no longer reduce the rendered frame width; they float over the block edges so the card can
  match the editor body's full horizontal span.
- The resource block now also exposes a visible selected state for center-click block selection, so image frames can be
  targeted as atomic structured blocks instead of behaving like untouchable display-only cards.
- That same whole-block focus chrome now stays visible while one of the transient left/right boundary caret anchors is
  active.
  Keyboard focus on a resource therefore still reads visually as "this block is the current token" instead of
  collapsing to one tiny caret fragment on the edge of the image frame.
- That center-selected state is now write-authoritative with respect to RAW deletion semantics:
  pressing `Backspace` or `Delete` while the block itself is selected dispatches one block-deletion request that
  removes the block's canonical `<resource ... />` source span from `.wsnbody`.
- Boundary caret anchors now also participate in atomic block deletion/navigation semantics:
  - left boundary caret: `Delete` removes the resource block, `Right` enters block selection
  - right boundary caret: `Backspace` removes the resource block, `Left` enters block selection
  Resource blocks therefore behave more like one cursor-addressable token during keyboard traversal instead of one
  isolated display card plus two unrelated empty editors.
- A center-selected resource token now consumes `Left` / `Right` itself before yielding to adjacent prose.
  Keyboard traversal therefore becomes:
  preceding prose -> left caret lane -> selected resource token -> right caret lane -> following prose
  instead of skipping straight from the selected image block to one neighboring text block.
- Those boundary anchors now keep one visible native caret at the resource row boundary while the same whole-block
  focus chrome remains active.
  Inline image/resource focus therefore reads like one selected attachment token with explicit before/after caret
  positions, which matches mainstream notes-app interaction more closely than a detached top/bottom insertion lane.
- That selected outline now uses a neutral panel border instead of Accent so the interactive block state remains
  visually consistent with the Figma image-frame chrome.
- Parent viewport background taps are now filtered through the structured document flow hit-test helpers before the
  fallback end-edit path runs.
  A center click on the image frame can therefore preserve the block's selected state instead of being cleared one turn
  later by the outer viewport.
