# `src/app/models/file/sync/WhatSonEditorRawPushController.hpp`

## Responsibility

Declares the editor-surface-to-RAW push trigger controller.

## Contract

- Owns the trigger policy for pushing an editor surface session back into canonical RAW note storage.
- Accepts three trigger types:
  - idle push after the LVRS session file has synced
  - modified-count push when the editor surface revision increases
  - note-departure push before the active note context is cleared or changed
- Treats a pending modified-count push as newer than a following idle sync trigger for the same pending slot, so the
  user's first edit is not overwritten by a stale sync-finished surface snapshot.
- Leaves canonical active-session-source ownership to `NoteEditorDocumentSession`, which updates that source as soon as
  a modified-count push is requested and makes stale idle or note-departure pushes persist the session source instead
  of the older payload.
- Exposes `discardPendingPushForFile(...)` for authoritative writes such as imported-resource paste, where a pre-paste
  modified-count payload must not run after the canonical resource line has already been saved.
- Exposes `setRawPushCallback(...)`; the callback performs the actual RAW conversion and note-package write.
- Emits `rawPushRequested(...)` and `rawPushFinished(...)` for regression and diagnostics.

## Boundary

- This object is sync orchestration only.
- It must not parse `.wsnbody`, serialize body XML, mutate note headers, or know about note-package layout.
- `NoteEditorDocumentSession` remains the owner of editor document projection and RAW persistence.

## Tests

- `test/cpp/suites/editor_raw_push_controller_tests.cpp`
- `test/cpp/suites/note_editor_document_session_tests.cpp`
