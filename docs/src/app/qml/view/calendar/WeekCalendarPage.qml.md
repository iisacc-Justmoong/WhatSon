# `src/app/qml/view/calendar/WeekCalendarPage.qml`

## Role
`WeekCalendarPage.qml` renders a week timeline overlay using a standard calendar-grid pattern:
day headers across the top and hourly rows down the page.

## View Contract
- Input: `weekCalendarViewModel`
- Hook signal: `viewHookRequested(string reason)`
- Hook forwarder: `requestViewHook(reason)` delegates to `weekCalendarViewModel.requestWeekView(reason)`

## UI Composition
- Header:
  - previous/today/next week controls,
  - active week-range label.
- Timeline body:
  - left time axis (`HH:mm`) for 24 hours,
  - seven day columns with per-hour slot summaries,
  - highlighted current-day column.

## Interaction/Data Flow
1. `Component.onCompleted` requests `page-open`.
2. Header controls mutate week cursor (`shiftWeek`, `setDisplayedWeekStartIso`).
3. Hour-slot cells are derived in QML from `dayModels[*].entries`.
4. Cell summaries compress multiple entries into one-line labels.

## Collaborators
- `src/app/viewmodel/calendar/WeekCalendarViewModel.hpp/.cpp`
- `src/app/qml/view/panels/ContentViewLayout.qml`
