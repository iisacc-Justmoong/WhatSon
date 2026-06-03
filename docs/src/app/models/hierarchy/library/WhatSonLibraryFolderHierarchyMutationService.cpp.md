# `src/app/models/hierarchy/library/WhatSonLibraryFolderHierarchyMutationService.cpp`

## Responsibility

This file applies persistent library-folder mutations for the folder tree only. Note-header binding rewrites were
removed with the deleted note package persistence layer.

## UUID Rewrite Strategy

The service no longer depends on path remapping alone.

1. Build a lookup from the original tree keyed by folder UUID.
2. Build a second lookup from the staged tree keyed by the same UUIDs.
3. Preserve note records as supplied by the caller.

This means a rename or reparent mutation preserves folder identity in the staged tree without mutating note headers.

## Header Rewrite Logic

- Stored note headers are not read or written by this service.
- Existing `<folder uuid="...">path</folder>` bindings are preserved when they still resolve.
- Legacy headers without UUIDs still work through a path fallback during migration.
- Explicit UUID/full-path bindings are preserved even when one bound folder is the ancestor of
  another. Only legacy leaf-only context tokens are collapsed when they simply identify a nested
  descendant folder.
- UUID equality alone is not treated as “already synchronized”. The serialized folder path must also
  match the staged tree, otherwise the header is rewritten.
- Header-only folder rewrites preserve existing `lastModified` and `modifiedBy` values.
- Folder path comparisons now run through the shared escaped-segment semantics, so one folder label
  containing a literal `/` is not mis-read as a parent/child path during note-header remapping.

## Persistence Order

1. Calculate staged header rewrites from original-tree UUIDs to staged-tree UUID targets.
2. Write the staged folder tree file.
3. Return the caller-provided `LibraryNoteRecord` values unchanged.

This order keeps the sidebar tree and note metadata aligned even when a folder subtree is renamed by
changing one ancestor label.
