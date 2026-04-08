# `src/app/qml/view/calendar/AgendaPage.qml`

## Role
`AgendaPage.qml` renders the Agenda calendar route with date navigation, location summary, all-day events,
timed events, and agenda-item completion rows.

## View Contract
- Input: `agendaViewModel`
- Output signal: `noteOpenRequested(string noteId)`
- Hook signal: `viewHookRequested(string reason)`
- Hook forwarder: `requestViewHook(reason)` delegates to `agendaViewModel.requestAgendaView(reason)`

## UI Composition
- Surface:
  - root page keeps `color: "transparent"`.
- Header:
  - shared `CalendarTodayControl` (`Prev/Today/Next`) drives date cursor movement.
  - date label and location caption are rendered in the top row.
- Body sections:
  - `All day`
  - `Timed`
  - `Agenda` (checkbox-like agenda-item toggle interactions)

## Interaction Flow
1. `Component.onCompleted` requests `page-open`.
2. Header control actions mutate date cursor (`shiftDay`, `setDisplayedDateIso`) and request hooks.
3. Agenda-item toggles call `agendaViewModel.toggleAgendaItemCompleted(...)`.
4. Note lifecycle projections from the shared calendar board flow into the `All day` / `Timed` sections as ordinary
   event rows, while tasks remain in `Agenda`.
5. Tapping a projected note row in `All day` or `Timed` emits `noteOpenRequested(noteId)` so the host can reopen that
   note in the editor surface.
6. Empty sections show dedicated placeholder labels.

## Tests
- Automated test files are not currently present in this repository.
- Regression checklist:
    - Agenda header must keep date navigation and the location caption without rendering a secondary weather card.
    - Agenda-item toggles must continue to call `agendaViewModel.toggleAgendaItemCompleted(...)`.
    - Projected note entries with `allDay == true` must stay in the `All day` section instead of disappearing from the
      Agenda route.
    - Clicking or tapping a projected note row in `All day` or `Timed` must emit `noteOpenRequested(...)`.
    - Empty all-day/timed/agenda sections must continue to show their placeholder labels.

## Collaborators
- `src/app/viewmodel/calendar/AgendaViewModel.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml`
