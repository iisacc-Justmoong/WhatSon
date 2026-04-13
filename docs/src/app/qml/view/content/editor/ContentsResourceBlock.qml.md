# `src/app/qml/view/content/editor/ContentsResourceBlock.qml`

## Responsibility
Renders one structured document-flow block for a canonical `<resource ... />` source tag.

## Key Behavior
- Receives the parser-owned source span from `ContentsStructuredBlockRenderer`.
- Receives the resolved asset entry from `ContentsBodyResourceRenderer` when that resolver can map the
  `.wsresource` package reference to the real payload file inside the package bundle.
- Falls back to a metadata-only `ContentsResourceRenderCard` payload when the asset has not resolved yet, so the body
  still keeps a visible block slot instead of collapsing back into plain text.
- Implements the same `applyFocusRequest(...)` and `shortcutInsertionSourceOffset()` contract as other structured
  delegates, which lets the document-flow host keep block focus and shortcut insertion deterministic across reparses.
- A left click on the resource block no longer traps focus inside the block-only frame.
  Instead it emits `documentEndEditRequested()`, allowing the host to reopen plain text input at the document tail so
  typing can continue after a terminal resource block.
