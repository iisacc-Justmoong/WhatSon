# `src/app/file/note/WhatSonHubNoteCreationService.cpp`

## Responsibility

This implementation creates the initial note scaffold on disk and returns the matching
`LibraryNoteRecord`.

## UUID-Aware Note Creation

- The service aligns `assignedFolders` with `assignedFolderUuids`.
- It writes both values into `WhatSonNoteHeaderStore` through `setFolderBindings(...)`.
- It mirrors the same bindings into the returned `LibraryNoteRecord`.

## Result

A note created while a folder is selected can later survive parent-folder rename and move operations
because the note was born with the correct stable folder UUID, not merely the visible path string.
