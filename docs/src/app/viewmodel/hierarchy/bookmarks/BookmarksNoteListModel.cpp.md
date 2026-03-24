# `src/app/viewmodel/hierarchy/bookmarks/BookmarksNoteListModel.cpp`

## Responsibility

This implementation mirrors the library note-list model behavior for the bookmark domain: sanitize
incoming rows, derive fallback searchable text, filter by search text, preserve selection by note
id, and sort the visible source cache by newest modification time first.

## Sorting Pipeline

The bookmark note list uses the same stable descending ordering as the library note list:

1. `lastModifiedAt`
2. `createdAt`
3. original relative order for ties

That keeps the bookmark list deterministic while still promoting the note that was modified most
recently.

## Source Metadata
- Source path: `src/app/viewmodel/hierarchy/bookmarks/BookmarksNoteListModel.cpp`
- Source kind: C++ implementation
- File name: `BookmarksNoteListModel.cpp`
- Approximate line count: 631

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: no

### Classes and Structs
- `ValidationIssue`

### Enums
- None detected during scaffold generation.

## Verification

The direct model regression coverage remains in `tests/app/test_hierarchy_viewmodels.cpp`.
