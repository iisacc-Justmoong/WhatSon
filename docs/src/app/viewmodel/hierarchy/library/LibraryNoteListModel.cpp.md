# `src/app/viewmodel/hierarchy/library/LibraryNoteListModel.cpp`

## Responsibility

This implementation sanitizes incoming note-list items, derives searchable text fallbacks, applies
search filtering, preserves the selected note by id across resets, and now performs the canonical
latest-modified-first sort for library notes.

The model now keeps the normalized `bodyText` payload on each row instead of clearing it after
sanitization. That preserves the selected-note body contract exposed through `currentBodyText` and
prevents the editor from falling back to an empty document when the lazy note-body reload path does
not win the selection race.

Each row now also keeps a normalized `noteDirectoryPath`, and the selected-row contract exposes that
value through `currentNoteDirectoryPath`.
The list model therefore no longer describes the selected note purely by `noteId`; downstream
selection/mount code can keep following the concrete `.wsnote` package that backs the visible row.

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

`applySearchFilter()` still restores `m_currentIndex` by the previously selected logical note, and
the selected-row notifications now also include `currentNoteDirectoryPathChanged()` when the visible
package path changes.
This keeps the same concrete `.wsnote` package active when duplicate ids exist across multiple note
packages.

The same reset path now also emits `currentNoteEntryChanged()` whenever the filtered selection
materializes for the first time or the selected row is replaced in-place at the same visible index.
That keeps `currentNoteEntry` consumers aligned with the authoritative row selection instead of
depending on `currentIndexChanged()` alone.

`currentBodyText` is again a real selected-note payload. After search/filter resets the model still
restores `m_currentIndex` by note id, and the selected row continues to expose its normalized note
body through both `currentBodyText` and `BodyTextRole`.

The `applySearchFilter()` trace now logs post-reset `m_items` state instead of the moved-from
temporary filter buffer. The `nextCount`, `nextItemId`, and `nextItemDirectoryPath` fields therefore
describe the actual visible list state that downstream selection debugging should inspect.

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
When duplicate note ids exist, also confirm that the selected row keeps exporting the correct
`noteDirectoryPath` for the mounted package.
When the first visible selection materializes or the selected row is replaced by a reset, also
confirm that `currentNoteEntryChanged()` fires exactly once with the new row payload.
