# `src/app/models/file/hierarchy/library/LibraryDraft.hpp`

## Responsibility

`LibraryDraft` owns the derived "Draft" smart bucket.

## Public Contract

- `rebuild(...)`: rebuild the draft bucket from the canonical all-notes vector.
- `matches(...)`: centralize the draft-membership rule used by both rebuild and incremental mutation paths.
- `upsertNote(...)`: reevaluate one note against that rule and update the bucket in place.
- `removeNoteById(...)`: prune one draft note without rebuilding the whole bucket.

## Tests
- Automated test files are not currently present in this repository.
- Regression checklist:
  - body-only note saves that do not affect draft membership must not force a full `Draft` bucket rebuild
  - removing draft membership must route through `removeNoteById(...)` when the updated note no longer matches
