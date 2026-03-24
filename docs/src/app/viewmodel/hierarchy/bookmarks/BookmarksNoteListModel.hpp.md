# `src/app/viewmodel/hierarchy/bookmarks/BookmarksNoteListModel.hpp`

## Responsibility

`BookmarksNoteListModel` is the bookmark-domain peer of `LibraryNoteListModel`. It exposes the same
delegate-facing row roles, and it now carries internal `createdAt` / `lastModifiedAt` fields on
each `BookmarksNoteListItem` so the bookmark list can follow the same most-recently-modified-first
ordering policy as the main library list.

## Ordering Contract

- Rows are ordered by `lastModifiedAt` descending.
- `createdAt` is used when `lastModifiedAt` is missing.
- Ties preserve incoming relative order.

The bookmark sidebar therefore does not depend on hub parse order when multiple bookmarked notes are
visible together.

## Source Metadata
- Source path: `src/app/viewmodel/hierarchy/bookmarks/BookmarksNoteListModel.hpp`
- Source kind: C++ header
- File name: `BookmarksNoteListModel.hpp`
- Approximate line count: 116

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: yes

### Classes and Structs
- `BookmarksNoteListItem`
- `BookmarksNoteListModel`

### Enums
- `Role`

## Runtime Notes

Selection is still public as a visible row index, but the implementation restores selection by note
id after filter or resort operations.
