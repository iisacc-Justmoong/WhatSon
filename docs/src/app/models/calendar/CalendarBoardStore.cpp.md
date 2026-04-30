# `src/app/calendar/CalendarBoardStore.cpp`

## Implementation Notes
- Constructor now initializes the `ICalendarBoardStore` base.
- Manual event/task entries still remain mutable through `addEvent(...)`, `addTask(...)`, `removeEntry(...)`, and
  `setTaskCompleted(...)`.
- Manual board entries now maintain per-date entry and count indexes, and add/remove/complete paths update those
  indexes incrementally instead of rebuilding the full manual board cache.
- The store now keeps a second, read-only projection for library notes. That projection can be refreshed either from
  the loaded library runtime snapshot or by reindexing the current `.wshub` package from disk.
- Projected notes are now indexed twice: by `sourceId` for single-note mutation routing and by calendar date for fast
  day/month/year queries.
- `upsertProjectedNote(...)` and `removeProjectedNoteBySourceId(...)` now update one projected note mount without
  forcing a full projection rebuild.
- Calendar queries now also have a live-provider fallback, so `entriesForDate(...)` / `countsForDate(...)` can still
  surface projected note items when the explicit projected cache is empty but the library runtime snapshot is already
  available.
- Each note now contributes exactly one projected calendar entry.
- The projected date/time is chosen from whichever of `createdAt` or `lastModifiedAt` is more recent.
- The projected title now uses the same top-line preview text rule as `NoteListItem.primaryText`, instead of adding
  `Created note` / `Modified note` lifecycle prefixes to the chip label.
- Projected note entries stay on the shared `entriesForDate(...)` / `countsForDate(...)` path, so day/week/month/year
  controllers and Agenda all receive note items alongside manual calendar events.

## Tests
- Automated test files are not currently present in this repository.
- Regression checklist:
  - a startup-loaded library runtime snapshot containing a note whose `lastModifiedAt` is `2026-04-08-...` must
    produce a projected entry for `entriesForDate("2026-04-08")` without requiring a second disk reindex
  - if the projected cache is empty but the live library note provider returns notes for `2026-04-08`, month/day/week
    calendar queries must still receive those projected note items
  - adding, removing, or completing one manual calendar entry must update only that date bucket and must not require a
    full manual-board scan
  - a note whose `lastModifiedAt` is `2026-04-08-...` must appear in `entriesForDate("2026-04-08")`
  - a single library note save/create/delete must be representable through `upsertProjectedNote(...)` /
    `removeProjectedNoteBySourceId(...)` without forcing a full projected snapshot replacement
  - manual event/task entries must remain queryable after note projection reloads
  - deleting or completing a manual entry must not mutate read-only projected note entries
  - a note with both `createdAt` and `lastModifiedAt` must contribute only one projected entry, using the more recent
    timestamp
  - the projected entry title for a note with body preview text must match the note-list top-line preview headline
