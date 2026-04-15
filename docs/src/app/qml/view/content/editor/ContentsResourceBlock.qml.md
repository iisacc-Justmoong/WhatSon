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
- The block now treats the resource frame itself as a three-zone interaction surface, but only narrow edge hot zones
  on each side may leave whole-block selection:
  - left edge hot zone: move editing to the nearest preceding text block, or open a transient before-boundary editor
    when no preceding text block exists
  - center/default click area: select the resource block itself as one atomic document block
  - right edge hot zone: move editing to the nearest following text block, or open a transient after-boundary editor
    when no following text block exists
  Ordinary clicks on the attachment therefore stay on the same default path as mainstream notes apps: the block
  itself becomes selected unless the user intentionally targets one edge.
- Those transient boundary editors feed committed plain text back into RAW through
  `adjacentPlainTextInsertionRequested(side, text, cursorPosition)`.
  Typing on the left/right edge of an image block therefore materializes canonical prose before or after the
  `<resource ... />` tag instead of collapsing back to one fixed block-end insertion point.
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
- Those boundary anchors now keep their native text caret hidden and rely on whole-block focus chrome for visual state.
  Inline image/resource focus therefore reads like one selected attachment block rather than one tiny caret painted
  inside the image frame edge.
- That selected outline now uses a neutral panel border instead of Accent so the interactive block state remains
  visually consistent with the Figma image-frame chrome.
- Parent viewport background taps are now filtered through the structured document flow hit-test helpers before the
  fallback end-edit path runs.
  A center click on the image frame can therefore preserve the block's selected state instead of being cleared one turn
  later by the outer viewport.
