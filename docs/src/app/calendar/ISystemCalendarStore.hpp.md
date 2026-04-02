# `src/app/calendar/ISystemCalendarStore.hpp`

## Role
`ISystemCalendarStore` exposes locale and date-format state through an interface boundary.

## Contract
- Locale snapshot accessors: locale names, UI languages, time zone, date formats, first day of week
- Formatting: `snapshot`, `formatShortDate`, `formatNoteDate`
- Hooks: `refreshFromSystem`, `requestStoreHook`
- Signal: `systemInfoChanged`

## Notes
- Hierarchy viewmodels now observe this interface instead of a concrete store.
- Static fallback helpers remain on `SystemCalendarStore` as utility logic, not object communication.
