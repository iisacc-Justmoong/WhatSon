# `src/app/qml/view/calendar/DayCalendarPage.qml`

## Role
`DayCalendarPage.qml` renders the day calendar overlay with a 24-hour timeline pattern common in calendar apps.

## View Contract
- Input: `dayCalendarViewModel`
- Hook signal: `viewHookRequested(string reason)`
- Hook forwarder: `requestViewHook(reason)` delegates to `dayCalendarViewModel.requestDayView(reason)`

## UI Composition
- Header:
  - previous/today/next day navigation controls,
  - localized day label,
  - entry count summary.
- Body:
  - scrollable 24-hour slot timeline,
  - hour column (`HH:mm`) on the left,
  - entry cards grouped by hour on the right.

## Interaction Flow
1. `Component.onCompleted` requests `page-open`.
2. Header buttons mutate date cursor (`shiftDay`, `setDisplayedDateIso`) and request hooks.
3. Timeline binds directly to `timeSlots` from `DayCalendarViewModel`.

## Collaborators
- `src/app/viewmodel/calendar/DayCalendarViewModel.hpp/.cpp`
- `src/app/qml/view/panels/ContentViewLayout.qml`
