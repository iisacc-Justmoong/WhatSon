# `src/app/viewmodel/content/ContentsEditorSessionController.hpp`

## Role
- `ContentsEditorSessionController` is the C++ owner of note-editor session save/sync policy.
- The class is registered to QML through `WhatSonQmlInternalTypeRegistrar` so view QML can instantiate the C++
  controller directly without a non-view QML wrapper.

## QML Contract
- State properties kept on the controller:
  - `editorBoundNoteId`
  - `editorBoundNoteDirectoryPath`
  - `editorText`
  - `localEditorAuthority`
  - `lastLocalEditTimestampMs`
  - `pendingBodySave`
  - `typingIdleThresholdMs`
  - `selectionBridge`
  - `agendaBackend`
  - `syncingEditorTextFromModel`
- Invokable session operations exposed to QML:
  - `flushPendingEditorText()`
  - `isTypingSessionActive()`
  - `requestSyncEditorTextFromSelection(noteId, text, bodyNoteId, noteDirectoryPath = "")`
  - `markLocalEditorAuthority()`
  - `scheduleEditorPersistence()`
  - `persistEditorTextImmediately()`
  - `persistEditorTextImmediatelyWithText(text)`
- The controller emits `editorTextSynchronized()` after a model-owned note snapshot is accepted into the live editor
  session.

## Behavioral Notes
- The persisted RAW document remains the durable backing store, but once the selected note has
  `localEditorAuthority`, the live editor snapshot becomes authoritative for same-note reconciliation until RAW has
  converged to that editor text.
- The controller must therefore reject any differing same-note incoming model snapshot while local editor authority is
  still active, not only during the typing-idle protection window or while `pendingBodySave` is set.
- That same-note protection is now defined on the mounted package identity, not on `noteId` alone.
  If the incoming selection/session payload points at a different `.wsnote` directory for the same id, the controller
  must drop local-authority protection and rebind to the new package.
- That rejection must not itself enqueue another editor-to-RAW write.
- Persistence staging and immediate flush both normalize agenda placeholder dates and empty structured-block anchors
  before the payload enters `ContentsEditorSelectionBridge`.
- The controller owns the `syncingEditorTextFromModel` guard so QML typing handlers no longer need to schedule that
  guard window in JavaScript.
