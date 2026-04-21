# `src/app/models/file/note/WhatSonHubNoteCreationService.hpp`

## Responsibility

This header defines the service that scaffolds a new note inside a hub and returns a normalized
runtime record for immediate UI use.

## Folder Assignment Contract

`Request` now carries both:

- `assignedFolders`
- `assignedFolderUuids`

Callers creating a note from a selected library folder should provide both values so the new note is
born with a stable folder identity instead of receiving only a display path.

## Why This Matters

Creating a note inside a renamed subtree must not reintroduce path-based identity. The creation
service therefore writes the same UUID-aware folder binding contract that rename and drag-and-drop
mutations use.
