# `src/app/models/hierarchy/library/LibraryNotePreviewText.hpp`

## Responsibility

This header provides the shared note-preview text rules for library-domain consumers that need metadata-based labels
without reopening note storage at the view layer.

## Shared Preview Rules

- `notePrimaryText(...)` matches the note-list `primaryText` contract:
  - prefer `noteId`
  - fall back to `project`
  - fall back to the first non-empty folder label
- `notePrimaryHeadline(...)` extracts the first non-empty line from that same metadata payload so compact consumers such
  as calendar event chips can reuse the note-list headline text instead of inventing a second label rule.
- Callers are expected to pass fully normalized `LibraryNoteRecord` data.

## Tests

- `test/cpp/suites/library_note_list_model_tests.cpp` covers the metadata preview path.
- Regression checklist:
  - a note with `noteId` must prefer that id over project/folder fallback
  - `notePrimaryHeadline(...)` must return the visible top line of that same preview text
  - an empty preview payload must keep returning an empty string so downstream callers can decide their own fallback
