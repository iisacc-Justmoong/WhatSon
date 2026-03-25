# `src/app/file/hierarchy/library/WhatSonLibraryIndexedState.cpp`

## Implementation Summary

The implementation wraps three lower-level storage helpers:

- `LibraryAll`
- `LibraryDraft`
- `LibraryToday`

It keeps those helpers behind one backend API so higher layers can treat library note indexing as a
single responsibility.

## Derived Bucket Policy

`rebuildDerivedBuckets()` is the internal boundary that recomputes `draft` and `today` from the
canonical `all` notes collection. Callers that mutate notes only need to replace the canonical note
set once.

## Shared Reuse

`collectBookmarkedNotes(...)` provides the matching bookmark projection helper used by
`WhatSonRuntimeDomainSnapshots` and `BookmarksHierarchyViewModel`, which keeps bookmark derivation on
the already indexed library data instead of reparsing the hub.
