# `src/app/models/editor/bridge/ContentsEditorSelectionBridge.cpp`

## Runtime Behavior

The bridge keeps editor selection state synchronized with note-list and active content-controller
contracts. It creates one `ContentsNoteManagementCoordinator` and uses it for:

- content persistence contract availability;
- selected-note bind/clear events and open-count follow-up;
- lazy RAW body reads from the resolved `.wsnote` package;
- selected-note snapshot refresh;
- view-session versus filesystem RAW reconciliation.

The former pending-editor-body adoption path was removed with
`ContentsEditorPersistenceController`. Once an edit is accepted by the live editor session, direct save
coordination happens through `ContentsEditorSaveCoordinator`, not through this selection bridge.

## Selection Flow

- `currentNoteEntry`, `currentNoteId`, and row/current-index fallback continue to resolve the visible
  selected note, but only committed note identity contracts can own a body payload.
- `currentNoteDirectoryPath` is preserved as part of selection identity.
- One request sequence is tracked for asynchronous body loads, allowing stale worker completions to be
  ignored after a newer request or note change.
- When a package read cannot start, the bridge may still use `noteBodySourceTextForNoteId(...)` as a
  runtime snapshot fallback.

## Verification

Regression coverage asserts that the bridge references `ContentsNoteManagementCoordinator`, does not
reference `ContentsEditorPersistenceController`, and no longer contains the old
`selectionFlow.bodyLoadFromPendingEditor` trace.
