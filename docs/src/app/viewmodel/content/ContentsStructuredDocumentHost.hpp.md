# `src/app/viewmodel/content/ContentsStructuredDocumentHost.hpp`

## Responsibility
Declares the structured editor host object that bridges QML document-flow state into stable C++ properties and policy
entrypoints.

## Current Contract
- Stores normalized structured-flow host state:
  - document blocks
  - rendered resources
  - source text
  - active block index / cursor revision
  - pending focus request metadata
  - cached logical-line and block-layout summaries
  - viewport geometry
- Exposes the policy collaborators consumed by `ContentsStructuredDocumentFlow.qml`:
  - `collectionPolicy`
  - `focusPolicy`
  - `mutationPolicy`
- Exposes the selection-clear contract now used by structured block delegates:
  - `selectionClearRevision`
  - `selectionClearRetainedBlockIndex`
  - `requestSelectionClear(...)`
  - `noteActiveBlockInteraction(...)`, which handles focus/activation cleanup
  - `noteActiveBlockCursorInteraction(...)`, which updates cursor revision for live caret movement without clearing
    the current block's native selection

## Integration Note
- QML delegates should treat this host as the only structured selection-management authority.
- Delegates may preserve selection only for the currently retained block/editor; all other stale
  `persistentSelection` highlights must be cleared on the next revision tick.
- QML text delegates must route `TextEdit.cursorPosition` changes through `noteActiveBlockCursorInteraction(...)`, not
  focus activation, because desktop drag selection and iOS text selection gestures update the cursor repeatedly while
  the native `TextEdit` owns the active selection.
