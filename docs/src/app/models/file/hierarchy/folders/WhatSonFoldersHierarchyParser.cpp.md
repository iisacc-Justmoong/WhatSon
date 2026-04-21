# `src/app/models/file/hierarchy/folders/WhatSonFoldersHierarchyParser.cpp`

## Responsibility

This parser converts the persisted `Folders.wsfolders` structure into
`WhatSonFolderDepthEntry` rows. It also performs compatibility upgrades for older folder trees that
were saved before UUID support existed.

## Accepted UUID Keys

The parser accepts several field names when it reads a folder row:

- `uuid`
- `UUID`
- `folderUuid`

This keeps older experiments and partially migrated files readable without a manual data-cleanup
step.

## Upgrade Behavior

- If a folder row already carries a valid 64-character alphanumeric UUID, the parser preserves it.
- If the row has no UUID, or the UUID is invalid, the parser synthesizes a new one.
- Whenever that happens, `outUuidMigrationRequired` is set so the caller can persist the upgraded
  file immediately.

This makes UUID migration explicit instead of leaving the app with session-local random identities.

## Structural Output

Each parsed row returns:

- the legacy path id,
- the folder label,
- the tree depth,
- the stable UUID.

The parser therefore remains path-aware for readability while producing the runtime identity needed
for rename-safe mutations.

## Escaped Slash Canonicalization

- Parsed rows now treat `label` as the authoritative leaf name for one hierarchy level.
- During normalization, the parser rebuilds `entry.id` from `depth + parentPath + label` using the shared
  folder-path escaping rules.
- A literal `/` inside one label is therefore persisted as `\/` inside `entry.id` instead of spawning an accidental
  child hierarchy level.
- This also upgrades already-saved folder rows whose JSON still contains raw slash labels such as
  `"label": "Marketing/Sales"` with `"depth": 0`; they are re-emitted as one root node with canonical id
  `Marketing\/Sales`.
