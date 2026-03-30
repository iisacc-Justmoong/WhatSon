# `src/app/viewmodel/calendar/WeekCalendarViewModel.hpp`

## Role
`WeekCalendarViewModel` exposes locale-aware week timeline state for the editor calendar overlay.

## Core Surface
- `displayedWeekStartIso`: active week start date (`yyyy-MM-dd`).
- `weekLabel`: formatted week range label.
- `weekdayLabels`: ordered weekday headers from locale first-day-of-week.
- `dayModels`: seven day payload entries for the active week.

Each `dayModels` item carries:
- `dateIso`
- `day`
- `dayLabel`
- `isToday`
- `entries`
- `eventCount`
- `taskCount`
- `entryCount`

## Mutation and Hooks
- Slot:
  - `setDisplayedWeekStartIso(const QString&)`
- Invokables:
  - `shiftWeek(int deltaWeeks)`
  - `requestWeekView(const QString& reason = QString())`
  - `addEvent(dateIso, timeText, title, detail)`
  - `addTask(dateIso, timeText, title, detail)`
  - `entriesForDate(dateIso)`
  - `removeEntry(entryId)`
  - `setTaskCompleted(entryId, completed)`

## Board Integration
- `setCalendarBoardStore(CalendarBoardStore*)` binds the shared board backend.
- Board changes rebuild week models so per-day counts and entry lists stay synchronized.

## Signals
- `displayedWeekStartIsoChanged()`
- `weekViewChanged()`
- `weekViewRequested(QString reason)`
