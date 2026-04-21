# `src/app/models/file/hierarchy/library/LibraryNotePreviewText.hpp`

## Responsibility

This header provides the shared note-preview text rules for library-domain consumers that need to render note body
snippets without reparsing `.wsnote` content at the view layer.

## Shared Preview Rules

- `notePrimaryText(...)` matches the note-list `primaryText` contract:
  - prefer `bodyFirstLine`
  - append trimmed `bodyPlainText` when it does not already start with that first line
  - clamp the combined preview to the same five-line summary budget used by note cards
- `notePrimaryHeadline(...)` extracts the first non-empty line from that same preview payload so compact consumers such
  as calendar event chips can reuse the note-list headline text instead of inventing a second label rule.
- Callers are expected to pass fully normalized `LibraryNoteRecord` data.
  In particular, note-creation paths must populate `bodyFirstLine` and related body preview metadata with the same
  rules used by `WhatSonLocalNoteDocument.toLibraryNoteRecord()` so new notes do not momentarily render blank preview
  rows for data that already exists on disk.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - a note with `bodyFirstLine="Alpha"` and multi-line `bodyPlainText` must keep `notePrimaryText(...)` aligned with
    the note-list two-line preview contract
  - `notePrimaryHeadline(...)` must return the visible top line of that same preview text
  - a newly created note record must yield the same preview text as a later read-back of that same note document
  - an empty preview payload must keep returning an empty string so downstream callers can decide their own fallback
