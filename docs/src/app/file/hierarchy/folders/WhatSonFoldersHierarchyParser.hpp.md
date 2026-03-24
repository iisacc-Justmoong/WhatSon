# `src/app/file/hierarchy/folders/WhatSonFoldersHierarchyParser.hpp`

## Responsibility

This header declares the parser for persisted `Folders.wsfolders` content.

## Public Contract

`parse(...)` accepts raw folder text and fills `WhatSonFoldersHierarchyStore` with normalized
`WhatSonFolderDepthEntry` rows.

The modern contract also exposes `outUuidMigrationRequired`:

- `false` means every parsed row already had a valid folder UUID.
- `true` means the parser had to synthesize at least one UUID because the file was legacy or
  malformed.

Callers that own the source file path are expected to rewrite the folder file when migration is
required so UUID identity becomes durable across sessions.

## Main Collaborators

- `WhatSonFoldersHierarchyStore`: receives the normalized parsed rows.
- `LibraryHierarchyViewModel.cpp`: persists migrated UUIDs during direct library loads.
- `WhatSonRuntimeDomainSnapshots.cpp`: persists migrated UUIDs during startup snapshot loading.
