# `src/app/models/file/note/WhatSonNoteFolderBindingService.cpp`

## Responsibility

This file implements the canonical folder-binding behavior for notes.

## Core Rules

- Folder UUID is the first identity key when present.
- Path is still normalized and compared so stale visible text can be repaired.
- Duplicate bindings are removed while preserving first-writer order.
- Assigning a folder never discards unrelated existing bindings.

## Main Consumers

- `LibraryHierarchyViewModel` uses it for note drag/drop acceptance and assignment.
- `WhatSonLibraryFolderHierarchyMutationService` uses it when rewriting note headers after folder
  rename, move, or reorder.
- `WhatSonHubNoteFolderClearService` uses it to write an explicitly empty folder array.
