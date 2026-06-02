# `src/app/models/hierarchy/library/WhatSonLibraryFolderHierarchyMutationService.hpp`

## Responsibility

This header declares the service that commits persistent library-folder mutations. The service owns
the folder tree write boundary only.

## Why The Service Exists

Folder rename, move, delete, and reorder operations are not purely visual in this application:

- `Folders.wsfolders` changes the sidebar tree.
- note folder bindings are currently read-only runtime metadata.

The service no longer mutates note headers while the package persistence layer is deleted.

## UUID-Oriented Contract

The current implementation treats folder UUID as the canonical identity:

- path changes are allowed,
- UUIDs remain stable,
- note records are returned unchanged.

This solves the earlier failure mode where renaming a parent folder invalidated every descendant note
binding because the stored path no longer matched the runtime tree.
