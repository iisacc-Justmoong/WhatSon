# `src/app/models/file/sync/WhatSonEditorRawPullController.hpp`

## Responsibility

Declares the RAW-from-filesystem pull request controller for the note editor session.

## Contract

- Owns the sync-facing entrypoints for pulling canonical note RAW from the filesystem when a note is entered or opened.
- Accepts two trigger types:
  - note-entry pull
  - note-open pull
- Exposes `setRawPullCallback(...)`; the callback performs the actual note-package read and returns the queued load
  sequence.
- Emits `rawPullRequested(...)` and `rawPullFinished(...)` for regression and diagnostics.

## Boundary

- This object is sync orchestration only.
- It must not parse `.wsnbody`, project editor HTML, update open-count statistics, or write session files.
- `NoteEditorDocumentSession` remains the owner of editor projection and load-result application, while
  `ContentsNoteManagementCoordinator` remains the filesystem worker queue used by the callback.

## Tests

- `test/cpp/suites/editor_raw_pull_controller_tests.cpp`
- `test/cpp/suites/note_editor_document_session_tests.cpp`
