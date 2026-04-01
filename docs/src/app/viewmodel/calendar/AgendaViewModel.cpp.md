# `src/app/viewmodel/calendar/AgendaViewModel.cpp`

## Role
Implements Agenda list section projection (`all-day`, `timed`, `agendaItems`) and sample location/weather payload generation for
`AgendaViewModel`.

## Behavior Summary
- Initializes date cursor to `QDate::currentDate()`.
- Rebuilds Agenda payload when:
  - displayed date changes,
  - board store pointer changes,
  - board emits `entriesChanged()`,
  - `requestAgendaView(...)` is invoked.
- Emits debug traces for Agenda view requests under the `calendar.agenda` scope.

## Section Projection Rules
- `allDayEvents`: only `event` entries at `00:00`.
- `timedEvents`: `event` entries except all-day rows.
- `agendaItems`: all `task` entries with computed status labels.
- `summary`: per-section counters plus completed/remaining agenda-item counts.

## Weather/Location Objects
- Location is stored as a dedicated internal model and exposed as a QML-facing `QVariantMap`.
- Weather defaults are deterministic sample values derived from the selected date.
- Optional manual override is supported through `setWeather(...)`.

## Coverage
- `tests/app/test_agenda_viewmodel.cpp` validates defaults, section splitting, request signal emission, and
  completion toggling.
