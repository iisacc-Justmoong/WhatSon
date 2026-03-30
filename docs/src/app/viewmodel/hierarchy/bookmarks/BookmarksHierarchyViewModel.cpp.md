# `src/app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.cpp`

## Responsibility

`BookmarksHierarchyViewModel.cpp` projects bookmarked `LibraryNoteRecord` entries into bookmark
color folders plus the bookmark note-list model.

`loadFromWshub()` now resolves that source note set through `WhatSonLibraryIndexedState` and then
filters with `collectBookmarkedNotes(...)`, so bookmarks reuse the same backend library indexing
policy as the runtime loader and library hierarchy viewmodel.

## Hierarchy Metadata

`depthItems()` / `hierarchyModel()` now serialize an explicit `iconName` field for each bookmark
color row.

The current contract uses the shared `bookmarksbookmark` icon token for every bookmark color row.
It now also emits a per-row `iconSource` SVG override generated from
`WhatSonBookmarkColorPalette.hpp`, so the rendered bookmark glyph color matches the bookmark label
name even when QML palette helpers do not recolor the icon placeholder.

## Note List Projection

`buildBookmarksListItem(...)` now forwards `createdAt` and `lastModifiedAt` into
`BookmarksNoteListItem` in addition to the already-visible preview, folder, tag, image, and
bookmark-color roles.

That keeps the bookmark note-list model able to apply the same latest-modified-first ordering rule
used by the library list without inventing sort order from the localized `displayDate` string.

## Runtime Mirror Repair

- `reloadNoteMetadataForNoteId(...)` re-reads the active bookmarked note from disk and rebuilds the
  visible bookmark note row from the persisted `.wsnhead` values.
- If the refreshed header is no longer bookmarked, the note is dropped from `m_bookmarkedNotes`
  immediately so the bookmarks list stays aligned with the detail panel write path.

## Hierarchy Count Badge

- `depthItems()` now publishes `count` for every bookmark color row.
- The count value is computed from `m_bookmarkedNotes` by matching each note color against the row
  color token.
- Runtime note mutation paths that can alter color membership (`removeNoteById(...)`,
  `reloadNoteMetadataForNoteId(...)`) now emit `hierarchyModelChanged()` after the refresh so
  badge counts update immediately.

## Source Metadata
- Source path: `src/app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.cpp`
- Source kind: C++ implementation
- File name: `BookmarksHierarchyViewModel.cpp`
- Approximate line count: 722

## Extracted Symbols
- Declared namespaces present: yes
- QObject macro present: no

### Classes and Structs
- None detected during scaffold generation.

### Enums
- None detected during scaffold generation.

## Verification

Bookmark note-list ordering and selection behavior are covered by
`tests/app/test_hierarchy_viewmodels.cpp`.
