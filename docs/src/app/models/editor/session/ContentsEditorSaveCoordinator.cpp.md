# `src/app/models/editor/session/ContentsEditorSaveCoordinator.cpp`

## Runtime Behavior

This implementation removes the former editor persistence buffering layer from the save path.
When a view commits edited RAW source, the coordinator:

1. commits the RAW text into the mounted `ContentsEditorSessionController`;
2. marks the session as having a pending body save;
3. enqueues the RAW snapshot directly on `ContentsNoteManagementCoordinator`;
4. clears `pendingBodySave` only after `editorTextPersistenceFinished(...)` reports a successful
   same-note write for the exact session text.

The queue that remains is the note-management queue that performs actual `.wsnbody` mutation and
follow-up metadata/stat work. There is no separate editor-side dirty snapshot map, idle drain timer,
or pending editor body adoption path.

## Path Handling

- If the mounted editor session already knows `editorBoundNoteDirectoryPath`, direct writes target that
  package with `persistEditorTextForNoteAtPath(...)`.
- If the path is missing, the coordinator asks `ContentsNoteManagementCoordinator` to capture the current direct
  persistence context. If no concrete package path is available, the save is rejected rather than falling back to
  view-model body persistence.

## Verification

The C++ regression suite writes a temporary note through this coordinator and then reads the `.wsnbody`
package back through `WhatSonLocalNoteFileStore`, proving that a committed editor snapshot reaches the
filesystem RAW source without the removed persistence controller.
