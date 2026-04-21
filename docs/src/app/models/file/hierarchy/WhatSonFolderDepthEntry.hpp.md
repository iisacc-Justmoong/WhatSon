# `src/app/models/file/hierarchy/WhatSonFolderDepthEntry.hpp`

## Responsibility

`WhatSonFolderDepthEntry` is the compact in-memory representation of one persisted folder row.
It is used by folder parsers, folder serializers, hub bootstrap code, and the library hierarchy
viewmodel.

## Data Contract

- `id`: legacy full-path identifier such as `Research/Competitor`. The current code still keeps
  this field because persisted `.wsfolders` data and some fallback lookups are path-based.
- `label`: display name of the folder segment.
- `depth`: zero-based nesting depth used by tree reconstruction.
- `uuid`: stable 64-character alphanumeric folder identity. This is the canonical identity for
  runtime folder relationships after the UUID migration.

## Why Both `id` And `uuid` Exist

The repository is in a compatibility phase:

- `.wsfolders` still stores human-readable path information.
- note headers and runtime selection logic now prefer `uuid`.
- older data without `uuid` is upgraded by parsers and stores during load/save.

This split lets rename and reparent operations change visible paths without invalidating note-to-
folder bindings.

## Main Collaborators

- `WhatSonFoldersHierarchyParser.cpp`: reads rows from `.wsfolders`.
- `WhatSonFoldersHierarchyCreator.cpp`: writes rows back to disk.
- `LibraryHierarchyViewModel.cpp`: projects these rows into UI-facing hierarchy items.
- `WhatSonLibraryFolderHierarchyMutationService.cpp`: uses UUIDs to keep note headers aligned when
  folder paths change.
