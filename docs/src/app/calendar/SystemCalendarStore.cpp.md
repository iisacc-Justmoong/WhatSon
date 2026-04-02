# `src/app/calendar/SystemCalendarStore.cpp`

## Implementation Notes
- Constructor now initializes the `ISystemCalendarStore` base.
- Runtime locale refresh and formatting logic are unchanged.
- The patch only moves collaboration from concrete-type wiring to interface wiring.
