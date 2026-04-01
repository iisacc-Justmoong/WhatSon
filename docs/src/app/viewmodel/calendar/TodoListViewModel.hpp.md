# `src/app/viewmodel/calendar/TodoListViewModel.hpp`

## Role
`TodoListViewModel` exposes a Todo-focused day projection for the calendar surface. It is a QObject-backed viewmodel
that keeps Todo sections and weather/location cards in one MVVM contract.

## Core Surface
- `displayedDateIso`: active date cursor (`yyyy-MM-dd`).
- `dateLabel`: localized Todo header label.
- `location`: location object (`cityName`, `regionName`, `countryCode`, `timeZoneId`, `displayName`).
- `weather`: weather object (`conditionText`, `temperatureText`, `highLowText`, `precipitationText`, numeric fields).
- `allDayEvents`: event list where time is `00:00`.
- `timedEvents`: event list with explicit time slots.
- `tasks`: task list with completion/status metadata.
- `summary`: aggregate counters for all sections.

## Mutation and Hooks
- Slot:
  - `setDisplayedDateIso(const QString&)`
- Invokables:
  - `shiftDay(int deltaDays)`
  - `requestTodoListView(const QString& reason = QString())`
  - `addEvent(dateIso, timeText, title, detail)`
  - `addTask(dateIso, timeText, title, detail)`
  - `entriesForDate(dateIso)`
  - `removeEntry(entryId)`
  - `setTaskCompleted(entryId, completed)`
  - `toggleTaskCompleted(entryId)`
  - `setLocation(cityName, regionName, countryCode, timeZoneId)`
  - `setWeather(conditionText, temperatureCelsius, highCelsius, lowCelsius, precipitationPercent)`

## Board Integration
- `setCalendarBoardStore(CalendarBoardStore*)` binds the shared in-memory board backend.
- Board updates (`entriesChanged`) trigger Todo section rebuild.
- `addEvent` / `addTask` use `displayedDateIso` when date input is empty.

## Signals
- `displayedDateIsoChanged()`
- `todoListChanged()`
- `todoListRequested(QString reason)`

These signals form the contract consumed by `TodoListPage.qml` and overlay hosts.
