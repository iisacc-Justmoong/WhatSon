# `src/app/viewmodel/hierarchy/library/LibraryNoteListModel.cpp`

## Responsibility

This implementation sanitizes incoming note-list items, derives searchable text fallbacks, applies
search filtering, preserves the selected note by id across resets, and now performs the canonical
latest-modified-first sort for library notes.

## Sorting Pipeline

`setItems(...)` now normalizes `createdAt` and `lastModifiedAt` before the filtered source cache is
stored.

The model then performs a stable descending sort using:

1. `lastModifiedAt`
2. `createdAt`
3. original relative order for ties or invalid timestamps

The stable-sort requirement is important because notes with equal timestamps must not jitter between
refreshes.

## Selection Stability

`applySearchFilter()` still restores `m_currentIndex` by previously selected note id after every
reset. This keeps the same note active when a save updates `lastModifiedAt` and pushes that note to
the top of the list.

## Source Metadata
- Source path: `src/app/viewmodel/hierarchy/library/LibraryNoteListModel.cpp`
- Source kind: C++ implementation
- File name: `LibraryNoteListModel.cpp`
- Approximate line count: 628

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: no

### Classes and Structs
- `ValidationIssue`

### Enums
- None detected during scaffold generation.

## Verification

Validate library note-list ordering through runtime library-viewmodel synchronization checks.
