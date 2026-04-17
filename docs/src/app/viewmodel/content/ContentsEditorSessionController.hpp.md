# `src/app/viewmodel/content/ContentsEditorSessionController.hpp`

## Role
- `ContentsEditorSessionController` is the C++ owner of note-editor session save/sync policy that used to live in
  `ContentsEditorSession.qml`.
- The class is registered to QML through `WhatSonQmlInternalTypeRegistrar` and mounted behind the thin
  `ContentsEditorSession.qml` wrapper so existing editor call sites keep the same surface API.

## QML Contract
- State properties kept on the controller:
  - `editorBoundNoteId`
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
  - `requestSyncEditorTextFromSelection(noteId, text, bodyNoteId)`
  - `markLocalEditorAuthority()`
  - `scheduleEditorPersistence()`
  - `persistEditorTextImmediately()`
  - `persistEditorTextImmediatelyWithText(text)`
- The controller emits `editorTextSynchronized()` after a model-owned note snapshot is accepted into the live editor
  session.

## Behavioral Notes
- RAW remains the write authority.
- The controller may reject a same-note incoming model snapshot while local typing protection or `pendingBodySave`
  still applies, but that rejection must not itself enqueue another editor-to-RAW write.
- Persistence staging and immediate flush both normalize agenda placeholder dates and empty structured-block anchors
  before the payload enters `ContentsEditorSelectionBridge`.
- The controller owns the `syncingEditorTextFromModel` guard so QML typing handlers no longer need to schedule that
  guard window in JavaScript.
