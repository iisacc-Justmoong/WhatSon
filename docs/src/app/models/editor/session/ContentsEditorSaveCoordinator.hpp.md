# `src/app/models/editor/session/ContentsEditorSaveCoordinator.hpp`

## Responsibility

`ContentsEditorSaveCoordinator` is the C++ session-side save hook used by editor display hosts.
It keeps the live `ContentsEditorSessionController` as the editable RAW source and forwards accepted
RAW snapshots directly into `ContentsNoteManagementCoordinator`.

## Contract

- `commitEditedSourceText(text)` updates the live editor session through
  `ContentsEditorSessionController::commitRawEditorTextMutation(...)`, then immediately enqueues the
  resulting RAW snapshot for note-package persistence.
- `scheduleEditorPersistence()` and `persistEditorTextImmediately...()` now share the same direct
  note-management write path. There is no editor-owned buffered persistence controller between the
  session and `.wsnbody` file store.
- `syncEditorSessionFromSelection(...)` still flushes a pending session before rebinding to a different
  note identity, so note switches do not abandon unsaved RAW text.
- The coordinator owns only editor-session save routing. Concrete `.wsnote/.wsnbody` IO, stat refresh,
  open-count updates, and runtime mirror updates remain in `ContentsNoteManagementCoordinator`.

## Regression Notes

- The removed `src/app/models/editor/persistence/ContentsEditorPersistenceController.*` files must not
  return.
- Direct saves with a resolved `noteDirectoryPath` must call
  `ContentsNoteManagementCoordinator::persistEditorTextForNoteAtPath(...)`.
- If the exact path is not yet known, the coordinator must first capture it through
  `ContentsNoteManagementCoordinator::captureDirectPersistenceContextForNote(...)`. If no path can be
  captured, the save request is rejected instead of falling back to view-model body persistence.
