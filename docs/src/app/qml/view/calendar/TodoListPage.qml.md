# `src/app/qml/view/calendar/TodoListPage.qml`

## Role
`TodoListPage.qml` renders the Todo calendar route with date navigation, weather/location summary, all-day events,
timed events, and task completion rows.

## View Contract
- Input: `todoListViewModel`
- Hook signal: `viewHookRequested(string reason)`
- Hook forwarder: `requestViewHook(reason)` delegates to `todoListViewModel.requestTodoListView(reason)`

## UI Composition
- Surface:
  - root page keeps `color: "transparent"`.
- Header:
  - shared `CalendarTodayControl` (`Prev/Today/Next`) drives date cursor movement.
  - date label and location caption are rendered in the top row.
- Weather card:
  - `temperatureText`, `conditionText`, `highLowText`, and precipitation summary.
- Body sections:
  - `All day`
  - `Timed`
  - `Tasks` (checkbox-like task toggle interactions)

## Interaction Flow
1. `Component.onCompleted` requests `page-open`.
2. Header control actions mutate date cursor (`shiftDay`, `setDisplayedDateIso`) and request hooks.
3. Task toggles call `todoListViewModel.toggleTaskCompleted(...)`.
4. Empty sections show dedicated placeholder labels.

## Collaborators
- `src/app/viewmodel/calendar/TodoListViewModel.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml`
