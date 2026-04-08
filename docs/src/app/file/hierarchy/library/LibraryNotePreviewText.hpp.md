# `src/app/file/hierarchy/library/LibraryNotePreviewText.hpp`

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

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - a note with `bodyFirstLine="Alpha"` and multi-line `bodyPlainText` must keep `notePrimaryText(...)` aligned with
    the note-list two-line preview contract
  - `notePrimaryHeadline(...)` must return the visible top line of that same preview text
  - an empty preview payload must keep returning an empty string so downstream callers can decide their own fallback
