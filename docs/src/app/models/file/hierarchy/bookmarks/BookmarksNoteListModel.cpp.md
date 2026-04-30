# `src/app/models/file/hierarchy/bookmarks/BookmarksNoteListModel.cpp`

## Responsibility

This implementation mirrors the library note-list model behavior for the bookmark domain: sanitize
incoming rows, derive fallback searchable text, filter by search text, preserve selection by note
id, and sort the visible source cache by newest modification time first.

It now also clears stored `bodyText` after searchable-text normalization so bookmark rows do not
retain full note bodies in memory. Selected note bodies are opened lazily by the editor selection
bridge instead.

## Sorting Pipeline

The bookmark note list uses the same stable descending ordering as the library note list:

1. `lastModifiedAt`
2. `createdAt`
3. original relative order for ties

That keeps the bookmark list deterministic while still promoting the note that was modified most
recently.

`setItems(...)` now also short-circuits before replacing the normalized source cache when the
incoming bookmark payload is identical to the current one, so redundant bookmarked-note refreshes do
not trigger another reset/selection replay cycle.

`currentBodyText` therefore remains as a compatibility property only; ordinary bookmark rows now
report an empty body payload.

## Source Metadata
- Source path: `src/app/models/file/hierarchy/bookmarks/BookmarksNoteListModel.cpp`
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

Validate bookmark note-list ordering and projection behavior through runtime hierarchy/list interaction checks.

Also confirm that selecting a very large bookmarked note no longer depends on the bookmark list
holding that note's full body text in memory.
