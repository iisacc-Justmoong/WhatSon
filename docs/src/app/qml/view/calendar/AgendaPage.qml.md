# `src/app/qml/view/calendar/AgendaPage.qml`

## Role
`AgendaPage.qml` renders the Agenda calendar route with date navigation, weather/location summary, all-day events,
timed events, and agenda-item completion rows.

## View Contract
- Input: `agendaViewModel`
- Hook signal: `viewHookRequested(string reason)`
- Hook forwarder: `requestViewHook(reason)` delegates to `agendaViewModel.requestAgendaView(reason)`

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
  - `Agenda` (checkbox-like agenda-item toggle interactions)

## Interaction Flow
1. `Component.onCompleted` requests `page-open`.
2. Header control actions mutate date cursor (`shiftDay`, `setDisplayedDateIso`) and request hooks.
3. Agenda-item toggles call `agendaViewModel.toggleAgendaItemCompleted(...)`.
4. Empty sections show dedicated placeholder labels.

## Collaborators
- `src/app/viewmodel/calendar/AgendaViewModel.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml`
