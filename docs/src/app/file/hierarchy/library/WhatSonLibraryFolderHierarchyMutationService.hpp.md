# `src/app/file/hierarchy/library/WhatSonLibraryFolderHierarchyMutationService.hpp`

## Responsibility

This header declares the service that commits persistent library-folder mutations. The service owns
the write order between the visible folder tree and note-header folder bindings.

## Why The Service Exists

Folder rename, move, delete, and reorder operations are not purely visual in this application:

- `Folders.wsfolders` changes the sidebar tree.
- `.wsnhead` folder bindings determine which notes appear under each folder.

The service keeps those two layers synchronized as one mutation boundary.

## UUID-Oriented Contract

The current implementation treats folder UUID as the canonical identity:

- path changes are allowed,
- UUIDs remain stable,
- note headers are rewritten by UUID lookup against the staged hierarchy.

This solves the earlier failure mode where renaming a parent folder invalidated every descendant note
binding because the stored path no longer matched the runtime tree.
