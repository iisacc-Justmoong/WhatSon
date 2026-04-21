# `src/app/models/file/hierarchy/library/LibraryToday.hpp`

## Responsibility

`LibraryToday` owns the derived "Today" smart bucket.

## Public Contract

- `rebuild(...)`: rebuild the bucket from the canonical all-notes vector.
- `matches(...)`: centralize the date-membership rule used by both rebuild and incremental mutation paths.
- `upsertNote(...)`: re-evaluate one note against today's date and update the bucket in place.
- `removeNoteById(...)`: prune one note without a full today-bucket rebuild.

## Tests
- Automated test files are not currently present in this repository.
- Regression checklist:
  - saving a note whose effective today-membership does not change must not force a full `Today` bucket rebuild
  - changing a note across the today/non-today boundary must be representable through one `upsertNote(...)` call
