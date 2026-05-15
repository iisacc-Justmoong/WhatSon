# `src/app/models/file/sync/WhatSonEditorRawPullController.hpp`

## Responsibility

Declares the RAW-from-filesystem pull request controller for the note editor session.

## Contract

- Owns the sync-facing entrypoints for pulling canonical note RAW from the filesystem when a note is entered or opened.
- Accepts two trigger types:
  - note-entry pull
  - note-open pull
  - idle pull for the active opened note
- Exposes `idlePullIntervalMs`, defaulting to 5000 ms.
- Exposes `setActiveNoteForIdlePull(...)`, `clearActiveNoteForIdlePull()`, and `recordUserActivity()` so the editor
  session can treat a quiet active note as eligible for periodic filesystem refresh.
- Exposes `setRawPullCallback(...)`; the callback performs the actual note-package read and returns the queued load
  sequence.
- Emits `rawPullRequested(...)` and `rawPullFinished(...)` for regression and diagnostics.

## Boundary

- This object is sync orchestration only.
- It must not parse `.wsnbody`, project editor HTML, update open-count statistics, or write session files.
- `NoteEditorDocumentSession` remains the owner of editor projection and load-result application, while
  `ContentsNoteManagementCoordinator` remains the filesystem worker queue used by the callback.
- It does not choose the timestamp winner; it only requests the active note body. The session compares the returned
  filesystem timestamp against the loaded session timestamp before applying a pull.

## Tests

- `test/cpp/suites/editor_raw_pull_controller_tests.cpp`
- `test/cpp/suites/note_editor_document_session_tests.cpp`
