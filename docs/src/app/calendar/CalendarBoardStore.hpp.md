# `src/app/calendar/CalendarBoardStore.hpp`

## Role
`CalendarBoardStore` is the default in-memory implementation of `ICalendarBoardStore`.

## Interface Alignment
- Inherits `ICalendarBoardStore`.
- Keeps the same mutation/query surface while moving collaboration to the interface type.
- Emits the shared board signals declared on the interface.
- Maintains manual board entries and read-only projected note entries in the same query surface so calendar consumers do
  not need separate note-specific wiring.
- Exposes both snapshot-driven note projection refresh and `.wshub` reindex-based refresh so startup/runtime state can
  populate calendar notes immediately while non-library mutations can still fall back to disk reloads.
- Also accepts a live note provider so calendar queries can still resolve projected note items even if the explicit
  projected-entry cache has not been populated yet.
