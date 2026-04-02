# `src/app/calendar/SystemCalendarStore.hpp`

## Role
`SystemCalendarStore` is the concrete system-locale implementation behind `ISystemCalendarStore`.

## Interface Alignment
- Implements the full locale/date formatting contract from `ISystemCalendarStore`.
- Keeps static fallback formatting helpers for non-injected code paths.
- Serves as the concrete source injected into hierarchy viewmodels from `main.cpp`.
