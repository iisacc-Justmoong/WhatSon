# `src/app/qml/view/calendar/DayCalendarPage.qml`

## Role
`DayCalendarPage.qml` renders the day calendar overlay with a 24-hour timeline pattern common in calendar apps.

## View Contract
- Input: `dayCalendarController`
- Output signal: `noteOpenRequested(string noteId)`
- Hook signal: `viewHookRequested(string reason)`
- Hook forwarder: `requestViewHook(reason)` delegates to `dayCalendarController.requestDayView(reason)`

## UI Composition
- Surface:
  - root page keeps `LV.Theme.accentTransparent` so the app background shows through.
- Header:
  - shared `CalendarTodayControl` (`Prev/Today/Next`) only.
- Body:
  - adaptive 24-hour slot timeline that fills the remaining `ContentsView` height,
  - hour column (`HH:mm`) on the left,
  - entry cards grouped by hour on the right using shared `CalendarEventCell`,
  - slot height is computed as `(remainingHeight - spacing) / 24`, so `00:00` starts at the top row and the final slot reaches the bottom edge.

## Interaction Flow
1. `Component.onCompleted` requests `page-open`.
2. Header control actions mutate date cursor (`shiftDay`, `setDisplayedDateIso`) and request hooks.
3. Timeline binds directly to `timeSlots` from `DayCalendarController`.
4. Tapping a projected note chip resolves its `sourceId` and emits `noteOpenRequested(noteId)` so the host can reopen
   that note in the editor surface.

## Tests
- Automated test files are not currently present in this repository.
- Regression checklist:
    - Day-view note chips must stay visible in the correct projected slot.
    - Clicking or tapping a day-view note chip must emit `noteOpenRequested(...)`.
    - Manual event chips must remain display-only and must not pretend to open a library note.

## Collaborators
- `src/app/models/calendar/DayCalendarController.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/calendar/CalendarEventCell.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml`
