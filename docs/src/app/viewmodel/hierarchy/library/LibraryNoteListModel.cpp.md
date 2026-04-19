# `src/app/viewmodel/hierarchy/library/LibraryNoteListModel.cpp`

## Responsibility

This implementation sanitizes incoming note-list items, derives searchable text fallbacks, applies
search filtering, preserves the selected note by id across resets, and now performs the canonical
latest-modified-first sort for library notes.

The model now keeps the normalized `bodyText` payload on each row instead of clearing it after
sanitization. That preserves the selected-note body contract exposed through `currentBodyText` and
prevents the editor from falling back to an empty document when the lazy note-body reload path does
not win the selection race.

## Sorting Pipeline

`setItems(...)` now normalizes `createdAt` and `lastModifiedAt` before the filtered source cache is
stored.

Before the sanitized source cache is replaced, the model now compares the full normalized item list
against the existing source cache and returns early when nothing actually changed. That keeps
equivalent library refresh turns from forcing another full list reset.

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

`currentBodyText` is again a real selected-note payload. After search/filter resets the model still
restores `m_currentIndex` by note id, and the selected row continues to expose its normalized note
body through both `currentBodyText` and `BodyTextRole`.

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

Also confirm that the selected library note still exposes its body text immediately through the
note-list model after runtime snapshot refreshes and search/filter resets.
