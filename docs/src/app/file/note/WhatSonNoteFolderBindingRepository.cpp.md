# `src/app/file/note/WhatSonNoteFolderBindingRepository.cpp`

## Responsibility

This file implements folder-header I/O for note mutations.

## Behavior

- It resolves the effective `.wsnhead` path from a `LibraryNoteRecord` when needed.
- It reads note documents through `WhatSonLocalNoteFileStore`.
- It persists header-only updates so body content is untouched by folder operations.
- It can write either a fully prepared document or only a new folder-binding set.

## Why It Matters

Moving this code behind one repository prevents `LibraryHierarchyViewModel` and other callers from
re-implementing their own read/update sequences for folder metadata.
