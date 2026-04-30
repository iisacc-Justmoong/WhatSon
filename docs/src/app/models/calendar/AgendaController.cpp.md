# `src/app/models/calendar/AgendaController.cpp`

## Implementation Notes
- `entriesChanged` is now observed through `ICalendarBoardStore`.
- Agenda rebuild no longer tracks or compares any weather projection; only date label, location-independent section
  models, and summary counts participate in the derived state.
- `requestAgendaView(...)` is now hook/log-only; actual agenda rebuilding stays with `setDisplayedDateIso(...)`,
  `setCalendarBoardStore(...)`, and `entriesChanged`, which removes duplicate recomputation after header date changes.
- Agenda mutation commands are otherwise unchanged.
