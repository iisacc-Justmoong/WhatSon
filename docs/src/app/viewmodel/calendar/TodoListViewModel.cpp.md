# `src/app/viewmodel/calendar/TodoListViewModel.cpp`

## Role
Implements Todo list section projection (`all-day`, `timed`, `tasks`) and sample location/weather payload generation for
`TodoListViewModel`.

## Behavior Summary
- Initializes date cursor to `QDate::currentDate()`.
- Rebuilds Todo payload when:
  - displayed date changes,
  - board store pointer changes,
  - board emits `entriesChanged()`,
  - `requestTodoListView(...)` is invoked.
- Emits debug traces for Todo view requests under the `calendar.todo` scope.

## Section Projection Rules
- `allDayEvents`: only `event` entries at `00:00`.
- `timedEvents`: `event` entries except all-day rows.
- `tasks`: all `task` entries with computed status labels.
- `summary`: per-section counters plus completed/remaining task counts.

## Weather/Location Objects
- Location is stored as a dedicated internal model and exposed as a QML-facing `QVariantMap`.
- Weather defaults are deterministic sample values derived from the selected date.
- Optional manual override is supported through `setWeather(...)`.

## Coverage
- `tests/app/test_todo_list_viewmodel.cpp` validates defaults, section splitting, request signal emission, and
  completion toggling.
