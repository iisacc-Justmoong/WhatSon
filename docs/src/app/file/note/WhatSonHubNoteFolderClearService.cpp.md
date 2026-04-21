# `src/app/models/file/note/WhatSonHubNoteFolderClearService.cpp`

## Responsibility

This file clears a note's folder membership without touching body content.

## Execution Flow

1. Resolve the indexed note by `noteId`.
2. Read the `.wsnhead` document through `WhatSonNoteFolderBindingRepository`.
3. Persist an empty binding set through the same repository.
4. Synchronize the updated `LibraryNoteRecord` back into the returned vector.

The clear-folder path does not update `lastModified`/`modifiedBy` header fields because no
`.wsnbody` content changed.

## Why The Repository Matters

The clear-folder path now uses the same note-folder I/O boundary as drag/drop assignment and folder
tree mutation. That keeps all folder metadata writes on one code path.
