# `src/app/viewmodel/hierarchy/library/LibraryNoteListModel.hpp`

## Responsibility

`LibraryNoteListModel` is the concrete list-model contract that feeds the library note-list view.
It exposes only UI-facing roles such as `noteId`, `primaryText`, `bodyText`, `displayDate`,
folders, tags, and bookmark state, but each `LibraryNoteListItem` now also carries internal
`createdAt` and `lastModifiedAt` timestamps.

Those timestamps are not exported as QML roles. They exist so the model can keep the visible note
rows sorted by most recently modified note first while preserving the rest of the public delegate
contract.

## Ordering Contract

- `lastModifiedAt` is the primary ordering key.
- `createdAt` is the fallback ordering key when `lastModifiedAt` is empty or invalid.
- Items with no valid timestamp stay in their original relative order after timestamped items.

This means the library note list no longer depends on index-file append order or creation-time
insertion order when presenting the visible rows.

## Source Metadata
- Source path: `src/app/viewmodel/hierarchy/library/LibraryNoteListModel.hpp`
- Source kind: C++ header
- File name: `LibraryNoteListModel.hpp`
- Approximate line count: 116

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: yes

### Classes and Structs
- `LibraryNoteListItem`
- `LibraryNoteListModel`

### Enums
- `Role`

## Runtime Notes

- Selection is still exposed by visible row index because the QML surface is index-driven.
- Refresh-time selection recovery is performed by note id in the implementation, so resorting after
  a save keeps the same logical note selected even when its row moves.
