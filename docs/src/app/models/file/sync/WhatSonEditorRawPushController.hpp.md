# `src/app/models/file/sync/WhatSonEditorRawPushController.hpp`

## Responsibility

Declares the verified RAW-source push trigger controller.

## Contract

- Owns trigger ordering for pushing verified RAW source back into canonical note storage.
- Accepts three trigger types:
  - idle push after the LVRS session file has synced
  - modified-count push for fallback tests and legacy trigger callers
  - note-departure push before the active note context is cleared or changed
- Uses a nonzero default idle interval so a session-file sync notification does not run the push callback in the same
  turn that mounted or programmatically refreshed the editor document.
- Treats a pending modified-count RAW payload as newer than a following idle sync trigger for the same pending slot, so
  the user's first edit is not overwritten by a stale sync-finished snapshot.
- Leaves canonical active RAW source ownership to `NoteEditorDocumentSession`, which converts editor documents to RAW,
  rejects unsafe transient empty payloads, and supplies only validated source text to this controller.
- Exposes `discardPendingPushForFile(...)` for session-owned authoritative RAW mutations where an older pending payload
  must not run after the active source has advanced.
- Exposes `hasPendingPushForFile(...)` so idle filesystem pulls can detect local RAW write state before attempting a
  diff merge.
- Exposes `setRawPushCallback(...)`; the callback performs the actual note-package write for the already verified RAW
  source, with the editor session/coordinator carrying the loaded RAW base so persistence can apply a diff.
- Emits `rawPushRequested(...)` and `rawPushFinished(...)` for regression and diagnostics.

## Boundary

- This object is sync orchestration only.
- It must not parse `.wsnbody`, serialize body XML, mutate note headers, or know about note-package layout.
- `NoteEditorDocumentSession` remains the owner of editor document projection, editor-to-RAW conversion, and RAW
  persistence.

## Tests

- `test/cpp/suites/editor_raw_push_controller_tests.cpp`
- `test/cpp/suites/note_editor_document_session_tests.cpp`
