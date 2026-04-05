# `src/app/viewmodel/calendar/AgendaViewModel.cpp`

## Implementation Notes
- `entriesChanged` is now observed through `ICalendarBoardStore`.
- Agenda rebuild no longer tracks or compares any weather projection; only date label, location-independent section
  models, and summary counts participate in the derived state.
- Agenda mutation commands are otherwise unchanged.
