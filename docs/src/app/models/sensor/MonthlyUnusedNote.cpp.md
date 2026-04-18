# `src/app/models/sensor/MonthlyUnusedNote.cpp`

## Responsibility

Implements the monthly inactivity wrapper around `UnusedNoteSensorSupport`.

## Window Definition

- Captures one UTC reference time per refresh.
- Uses `referenceUtc.addMonths(-1)` as the monthly cutoff so the window follows calendar-month semantics rather than a
  fixed `30`-day approximation.
