# `src/app/calendar/CalendarBoardStore.hpp`

## Role
`CalendarBoardStore` is the default in-memory implementation of `ICalendarBoardStore`.

## Interface Alignment
- Inherits `ICalendarBoardStore`.
- Keeps the same mutation/query surface while moving collaboration to the interface type.
- Emits the shared board signals declared on the interface.
- Maintains manual board entries and read-only projected note entries in the same query surface so calendar consumers do
  not need separate note-specific wiring.
- Maintains per-date entry indexes and per-date count caches for both manual board entries and projected note entries,
  so `entriesForDate(...)` / `countsForDate(...)` do not have to rescan the full board on every query.
- Exposes both snapshot-driven note projection refresh and `.wshub` reindex-based refresh so startup/runtime state can
  populate calendar notes immediately while non-library mutations can still fall back to disk reloads.
- Exposes single-note projected note upsert/remove entry points so library runtime mutations can update one calendar
  note mount without replacing the whole projected snapshot.
- Also accepts a live note provider so calendar queries can still resolve projected note items even if the explicit
  projected-entry cache has not been populated yet.
