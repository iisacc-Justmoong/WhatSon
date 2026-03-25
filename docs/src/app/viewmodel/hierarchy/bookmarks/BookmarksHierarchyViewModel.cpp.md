# `src/app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.cpp`

## Responsibility

`BookmarksHierarchyViewModel.cpp` projects bookmarked `LibraryNoteRecord` entries into bookmark
color folders plus the bookmark note-list model.

## Hierarchy Metadata

`depthItems()` / `hierarchyModel()` now serialize an explicit `iconName` field for each bookmark
color row.

The current contract uses the shared `bookmarksbookmarksList` icon token so downstream consumers
such as the detail-panel combo popup can render the same icon the hierarchy advertises, instead of
falling back to a generic menu icon.

## Note List Projection

`buildBookmarksListItem(...)` now forwards `createdAt` and `lastModifiedAt` into
`BookmarksNoteListItem` in addition to the already-visible preview, folder, tag, image, and
bookmark-color roles.

That keeps the bookmark note-list model able to apply the same latest-modified-first ordering rule
used by the library list without inventing sort order from the localized `displayDate` string.

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
