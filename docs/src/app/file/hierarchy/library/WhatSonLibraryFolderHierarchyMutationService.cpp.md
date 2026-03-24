# `src/app/file/hierarchy/library/WhatSonLibraryFolderHierarchyMutationService.cpp`

## Responsibility

This file applies persistent library-folder mutations atomically enough for the current local-storage
model. It rewrites note headers first, writes the staged `Folders.wsfolders` tree second, and rolls
back header-only rewrites if a later step fails.

## UUID Rewrite Strategy

The service no longer depends on path remapping alone.

1. Build a lookup from the original tree keyed by folder UUID.
2. Build a second lookup from the staged tree keyed by the same UUIDs.
3. Resolve each note's assigned folders to canonical leaf UUIDs.
4. Rehydrate the note header with the staged folder paths that correspond to those UUIDs.

This means a rename or reparent mutation changes the path shown in the note header while preserving
the semantic folder identity.

## Header Rewrite Logic

- Stored note headers are read and written through `WhatSonNoteFolderBindingRepository`.
- Binding comparison is delegated to `WhatSonNoteFolderBindingService` so drag/drop and hierarchy
  rename use the same equality rules.
- Existing `<folder uuid="...">path</folder>` bindings are preserved when they still resolve.
- Legacy headers without UUIDs still work through a path fallback during migration.
- Redundant ancestor assignments are removed before the note is rewritten so headers keep only the
  canonical leaf folders.
- UUID equality alone is not treated as “already synchronized”. The serialized folder path must also
  match the staged tree, otherwise the header is rewritten.

## Persistence Order

1. Calculate staged header rewrites from original-tree UUIDs to staged-tree UUID targets.
2. Persist header-only note updates through `WhatSonNoteFolderBindingRepository`.
3. Write the staged folder tree file.
4. Synchronize returned `LibraryNoteRecord` values from the rewritten documents.

This order keeps the sidebar tree and note metadata aligned even when a folder subtree is renamed by
changing one ancestor label.
