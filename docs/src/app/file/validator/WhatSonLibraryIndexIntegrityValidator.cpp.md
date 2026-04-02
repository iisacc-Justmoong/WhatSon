# `src/app/file/validator/WhatSonLibraryIndexIntegrityValidator.cpp`

## Role
`WhatSonLibraryIndexIntegrityValidator` validates index/runtime consistency for library note records and rewrites index files from materialized records.

## Orphan Pruning Behavior
`pruneOrphanRecords(...)` now performs two integrity stages for each record:

1. Run `m_noteStorageValidator.normalizeWsnotePackage(record, &normalizationError)`.
   - This enforces the strict `.wsnote` package contract before existence checks.
   - Failures are traced with `notePackage.normalizeFailed`, but processing continues so valid records are not blocked by one bad package.
2. Evaluate `hasMaterializedStorage(record)`.
   - Records without header/directory materialization are pruned as orphans.
   - Orphan note IDs are accumulated in `PruneResult::prunedOrphanNoteIds`.

## Index Rewrite Behavior
`rewriteIndexesFromRecords(...)` rewrites each `index.wsnindex` from the filtered record set:
- Keeps only note IDs that still belong to each library root.
- Persists rewritten index text through `WhatSonLibraryHierarchyCreator` + `WhatSonLibraryHierarchyStore`.
- Returns the first write error via `errorMessage`.

## Why Normalization Happens Here
The library index validator is on the index integrity path, so placing normalization here guarantees:
- package pollution is removed before index materialization decisions,
- migrated/created required note files are visible to subsequent storage checks,
- orphan pruning reflects post-normalization state rather than stale filesystem leftovers.
