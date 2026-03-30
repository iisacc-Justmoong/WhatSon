# `src/app/viewmodel/calendar/DayCalendarViewModel.hpp`

## Role
`DayCalendarViewModel` exposes a single-day timeline model for the editor calendar overlay. It is a QObject-backed
viewmodel designed for direct QML binding.

## Core Surface
- `displayedDateIso`: active day cursor (`yyyy-MM-dd`).
- `dayLabel`: locale-formatted day title.
- `dayEntries`: flat list of entries for the selected day.
- `timeSlots`: 24-slot timeline payload (`hour`, `timeLabel`, `entries`, `entryCount`).

## Mutation and Hooks
- Slot:
  - `setDisplayedDateIso(const QString&)`
- Invokables:
  - `shiftDay(int deltaDays)`
  - `requestDayView(const QString& reason = QString())`
  - `addEvent(dateIso, timeText, title, detail)`
  - `addTask(dateIso, timeText, title, detail)`
  - `entriesForDate(dateIso)`
  - `removeEntry(entryId)`
  - `setTaskCompleted(entryId, completed)`

## Board Integration
- `setCalendarBoardStore(CalendarBoardStore*)` binds the shared in-memory board backend.
- Board updates (`entriesChanged`) trigger day timeline rebuild.
- `addEvent` / `addTask` use `displayedDateIso` when date input is empty.

## Signals
- `displayedDateIsoChanged()`
- `dayEntriesChanged()`
- `dayViewChanged()`
- `dayViewRequested(QString reason)`

These signals form the contract consumed by `DayCalendarPage.qml` and overlay hosts.
